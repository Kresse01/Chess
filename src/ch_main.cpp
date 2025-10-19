#include "chess/core/ch_bitboard.h"
#include <iostream>

static void print_bb(ch::BB b)
{
    for (int r = 7; r >= 0; --r)
    {
        for (int f = 0; f < 8; ++f)
        {
            std::cout << ((b>>((r<<3)|f)) & 1ull);
        }
        std::cout<<'\n';
    }
    std::cout<<"------\n";
}

int main()
{
    ch::init_bitboards();

    // Knight from d4 (f=3, r =3, -> s=27)
    print_bb(ch::KNIGHT_ATK[27]);
    // Bishop rays from c1 (s=2)
    ch::BB span = ch::ray_attacks_from(2,ch::NE, 0) |
                  ch::ray_attacks_from(2,ch::NW, 0) |
                  ch::ray_attacks_from(2,ch::SE, 0) |
                  ch::ray_attacks_from(2,ch::SW, 0);

    print_bb(span);
}