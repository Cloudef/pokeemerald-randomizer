//     Pokemon (GEN 3) Text Encoding
//   | 0 1 2 3 4 5 6 7 8 9 A B C D E F
//   ---------------------------------
// 0 |   À Á Â Ç È É Ê Ë Ì   Î Ï Ò Ó Ô
// 1 | Œ Ù Ú Û Ñ ß à á   ç è é ê ë ì
// 2 | î ï ò ó ô œ ù ú û ñ º ª ᵉ & +
// 3 |        Lv = ;
// 4 |
// 5 | ▯ ¿ ¡ PKMNPOKé      Í % ( )
// 6 |                 â             í
// 7 |                   ⬆ ⬇ ⬅ ➡ * * *
// 8 | * * * * ᵉ < >
// 9 |
// A | ʳᵉ0 1 2 3 4 5 6 7 8 9 ! ? . - ・
// B | … “ ” ‘ ’ ♂ ♀ $ , × / A B C D E
// C | F G H I J K L M N O P Q R S T U
// D | V W X Y Z a b c d e f g h i j k
// E | l m n o p q r s t u v w x y z ▶
// F | : Ä Ö Ü ä ö ü   C t r l c h r s

#define UNA "0x0A"
#define U18 "0x18"
#define U1F "0x1F"
#define U2F "0x2F"
#define U30 "0x30"
#define U31 "0x31"
#define U32 "0x32"
#define U33 "0x33"
#define U37 "0x37"
#define U38 "0x38"
#define U39 "0x39"
#define U3A "0x3A"
#define U3B "0x3B"
#define U3C "0x3C"
#define U3D "0x3D"
#define U3E "0x3E"
#define U3F "0x3F"
#define U40 "0x40"
#define U41 "0x41"
#define U42 "0x42"
#define U43 "0x43"
#define U44 "0x44"
#define U45 "0x45"
#define U46 "0x46"
#define U47 "0x47"
#define U48 "0x48"
#define U49 "0x49"
#define U4A "0x4A"
#define U4B "0x4B"
#define U4C "0x4C"
#define U4D "0x4D"
#define U4E "0x4E"
#define U4F "0x4F"
#define U58 "0x58"
#define U59 "0x59"
#define U5E "0x5E"
#define U5F "0x5F"
#define U60 "0x60"
#define U61 "0x61"
#define U62 "0x62"
#define U63 "0x63"
#define U64 "0x64"
#define U65 "0x65"
#define U66 "0x66"
#define U67 "0x67"
#define U69 "0x69"
#define U6A "0x6A"
#define U6B "0x6B"
#define U6C "0x6C"
#define U6D "0x6D"
#define U6E "0x6E"
#define U70 "0x70"
#define U71 "0x71"
#define U72 "0x72"
#define U73 "0x73"
#define U74 "0x74"
#define U75 "0x75"
#define U76 "0x76"
#define U77 "0x77"
#define U78 "0x78"
#define U87 "0x87"
#define U88 "0x88"
#define U89 "0x89"
#define U8A "0x8A"
#define U8B "0x8B"
#define U8C "0x8C"
#define U8D "0x8D"
#define U8E "0x8E"
#define U8F "0x8F"
#define U90 "0x90"
#define U91 "0x91"
#define U92 "0x92"
#define U93 "0x93"
#define U94 "0x94"
#define U95 "0x95"
#define U96 "0x96"
#define U97 "0x97"
#define U98 "0x98"
#define U99 "0x99"
#define U9A "0x9A"
#define U9B "0x9B"
#define U9C "0x9C"
#define U9D "0x9D"
#define U9E "0x9E"
#define U9F "0x9F"
#define UF7 "0xF7"
#define UF8 "0xF8"
#define UF9 "0xF9"
#define UFA "0xFA"
#define UFB "0xFB"
#define UFC "0xFC"
#define UFD "0xFD"
#define UFE "0xFE"
#define UFF "0xFF"

#define SLV "Lv"
#define SPK "PK"
#define SMN "MN"
#define SPO "PO"
#define SKE "KE"
#define SRE "ʳᵉ"
#define SMP "・"

static const char *table[] = {
   " ", "À", "Á", "Â", "Ç", "È", "É", "Ê", "Ë", "Ì", UNA, "Î", "Ï", "Ò", "Ó", "Ô",
   "Œ", "Ù", "Ú", "Û", "Ñ", "ß", "à", "á", U18, "ç", "è", "é", "ê", "ë", "ì", U1F,
   "î", "ï", "ò", "ó", "ô", "œ", "ù", "ú", "û", "ñ", "º", "ª", "ᵉ", "&", "+", U2F,
   U30, U31, U32, U33, SLV, "=", ";", U37, U38, U39, U3A, U3B, U3C, U3D, U3E, U3F,
   U40, U41, U42, U43, U44, U45, U46, U47, U48, U49, U4A, U4B, U4C, U4D, U4E, U4F,
   "▯", "¿", "¡", SPK, SMN, SPO, SKE, "é", U58, U59, "Í", "%", "(", ")", U5E, U5F,
   U60, U61, U62, U63, U64, U65, U66, U67, "â", U69, U6A, U6B, U6C, U6D, U6E, "í",
   U70, U71, U72, U73, U74, U75, U76, U77, U78, "⬆", "⬇", "⬅", "➡", "*", "*", "*",
   "*", "*", "*", "*", "ᵉ", "<", ">", U87, U88, U89, U8A, U8B, U8C, U8D, U8E, U8F,
   U90, U91, U92, U93, U94, U95, U96, U97, U98, U99, U9A, U9B, U9C, U9D, U9E, U9F,
   SRE, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "!", "?", ".", "-", SMP,
   "…", "“", "”", "‘", "’", "♂", "♀", "$", ",", "×", "/", "A", "B", "C", "D", "E",
   "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U",
   "V", "W", "X", "Y", "Z", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
   "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "▶",
   ":", "Ä", "Ö", "Ü", "ä", "ö", "ü", UF7, UF8, UF9, UFA, UFB, UFC, UFD, UFE, UFF,
};

#include "pokemon_text.h"
#include <stdbool.h>
#include <memory.h>

size_t pokemon_text_to_utf8(const uint8_t *input, const size_t input_sz, char *out_buf, const size_t out_buf_sz) {
   size_t out = 0;
   if (out_buf && out_buf_sz > 0) {
      for (size_t i = 0; i < input_sz && out < out_buf_sz - 1; ++i) {
         uint8_t chr = input[i];
         if (chr == 0xFF) break;
         const size_t len = strlen(table[chr]);
         memcpy(out_buf + out, table[chr], len);
         out += len;
      }
      out_buf[out < out_buf_sz ? out : out - 1] = 0;
   }
   return out;
}

size_t pokemon_utf8_to_text(const char *input, const size_t input_sz, uint8_t *out_buf, const size_t out_buf_sz) {
   size_t out = 0, in = 0;
   while (out < out_buf_sz && in < input_sz) {
      bool found = false;
      for (uint8_t i = 0; i < 0xFF; ++i) {
         const size_t len = strlen(table[i]);
         if (input_sz - in < len || memcmp(table[i], input + in, len)) continue;
         in += len;
         out_buf[out++] = i;
         found = true;
         break;
      }
      if (!found) break;
   }
   return out;
}
