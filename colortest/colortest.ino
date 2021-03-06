//#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>

const uint16_t PixelCount = 64;
const uint8_t PixelPin = 24; // ignored for ESP8266
const uint8_t whiteness = 170;
const char white = 8;
const char black = 16;

NeoPixelBrightnessBus<NeoGrbwFeature, Neo800KbpsMethod> strip(PixelCount,PixelPin);
NeoGamma<NeoGammaTableMethod> colorGamma;
//
//RgbwColor bpawn(128, 200, 0 , 90);
//RgbwColor wpawn(0, 255, 140, 0);
//
//RgbwColor bking(255, 0, 0, 0);
//RgbwColor wking(255, 30, 255, 100);
//
//RgbwColor bknight(0, 255, 0, 0);
//RgbwColor wknight(28, 102, 255, 0);
//
//RgbwColor bbishop(255, 255, 0, 0);
//RgbwColor wbishop(0, 200, 255, 0);
//
//RgbwColor brook(255, 170, 0, 0);
//RgbwColor wrook(179, 0, 119, 0);
//
//RgbwColor bqueen(200, 0, 0, 100);
//RgbwColor wqueen(128, 0, 200, 0);
//
//RgbwColor empty(0);
//RgbwColor none(0,0,0, 255);

RgbwColor bpawn(255, 255, 0 , 200);
RgbwColor wpawn(255, 255, 0, 0); //yellow

RgbwColor bking(0, 0, 255, 200);
RgbwColor wking(0, 0, 255, 0);//dark blue

RgbwColor bknight(255, 128, 0, 200);
RgbwColor wknight(255, 128, 0, 0);//orange

RgbwColor bbishop(255, 0, 0, 100);
RgbwColor wbishop(255, 0, 0, 0);//red

RgbwColor brook(0, 255, 0, 200);
RgbwColor wrook(0, 255, 0, 0); // green

RgbwColor bqueen(187, 51, 255, 100);
RgbwColor wqueen(187, 51, 255, 0);//purple

RgbwColor empty(0);
RgbwColor none(0,0,0, 255);


RgbwColor checker_square(0, 0, 0, 255);
RgbwColor piece_colors[] =   {empty, none, bpawn, bking, bknight, bbishop, brook, bqueen, 
                                none, wpawn, none, wking, wknight, wbishop, wrook, wqueen};

char pieces[] = ".-pknbrq-P-KNBRQ";
char board[129] = {
    22, 20, 21, 23, 19, 21, 20, 22, 0,  0,  0,  0,  0,  0,  0,  0,
    18, 18, 18, 18, 18, 18, 18, 18, 0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    9,  9,  9,  9,  9,  9,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0,
    14, 12, 13, 15, 11, 13, 12, 14, 0,  0,  0,  0,  0,  0,  0,  0,
};
const uint8_t move_offsets[] = {
    15,  16,  17,   0,                              // black pawns
    15, -16, -17,   0,                              // white pawns
     1,  16,  -1, -16,   0,                         // rooks
     1,  16,  -1, -16,  15, -15,  17, -17,   0,     // queens, kings and bishops
    14, -14,  18, -18,  31, -31,  33, -33,   0,     // knights
     3,  -1,  12,  21,  16,  7, 12                  // starting indices for each piece type in order
                                                    //   white pawns, black pawns, kings, knights, bishops, rooks, queens
};     
void print_board(){
    uint8_t i = 0;
    char piece = 0;
    while(i < 128){
        //on the board
        if((i & 0x88) == 0){
            piece = board[i];
            Serial.print(pieces[piece & 15]);
            Serial.print(' ');
            ++i;
        } else {
            Serial.println();
            i += 8;
        }
    }
    Serial.println();
}

void display_board(){
    uint8_t i = 0;
    char piece = 0;
    RgbwColor color;
    while(i < 128){
        //on the board
        if((i & 0x88) == 0){
            piece = board[i];
            color = piece_colors[piece & 15];
            uint8_t strip_i = strip_index(i);
            //adds checkers on empty squares
            if(color == empty){
                strip.SetPixelColor(strip_i, (strip_i%2)? RgbwColor::LinearBlend(color,checker_square,0.10):color);
            } else {
                strip.SetPixelColor(strip_i,color);
            }
            
            //without checkers
            
            ++i;
        } else i+=8;
    }
    strip.Show();
}

uint8_t strip_index(uint8_t index){
    uint8_t row = index / 16;
    uint8_t col = index % 16;
    return 8*row + ((row%2)?(7-col):col); //even rows are reversed
}


void search(char side){
    uint8_t index = 0;
    //loop over board squares
    while(index<128){
        if((index & 0x88) == 0){
            char piece = board[index];
            //if the piece is on the right side
            if(piece & side){
                char piece_type = piece & 7;
                //moves to the starting indices in offsets
                uint8_t directions = move_offsets[piece_type + 30];
                ++directions;

                //loop over more offsets
                while(move_offsets[directions]){
                    uint8_t step_vector = move_offsets[directions];
                    ++directions;
                    
                    uint8_t source_square = index;
                    uint8_t target_square = source_square;
                    uint8_t captured_piece = 0;

                    //loop over slider ray
                    while(captured_piece == 0){
                        target_square += step_vector;
                        uint8_t captured_square = target_square;

                        if(target_square & 0x88) break;//off board

                        captured_piece = board[captured_square];

                        if(captured_piece & side) break; //same side

                        if(piece_type < 3 && (!(step_vector & 7)) != !captured_piece) break;//pawn limitations

                        //make move
                        board[captured_square] = board[source_square] = 0;
                        board[target_square] = piece;

                        //pawn promotion
                        if(piece_type < 3){
                            if(target_square + step_vector + 1 & 0x80){
                                board[target_square] |= 7;
                            }
                        }
                        print_board();
                        display_board();
                        delay(1000);
                        //take back move
                        board[target_square] = 0;
                        board[source_square] = piece;
                        board[captured_square] = captured_piece;

                        //fake capture for leapers
                        captured_piece += (piece_type < 5);

                        //unfake capture for double pawn move
                        if(piece_type < 3 && (6 * side + (target_square & 0x70) == 0x80)){
                           --captured_piece; 
                        }             
                    }
                }
            }
        }
        ++index;
    }
}
void setup(){
    Serial.begin(115200);
    while (!Serial);

    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();
    for(uint8_t i=0; i<14; ++i){
        piece_colors[i]  = colorGamma.Correct(piece_colors[i]);
    }
    strip.SetBrightness(100);
    strip.Begin();
    display_board();
}

void loop(){
    delay(1000);
    search(white);
    search(black);    
}
