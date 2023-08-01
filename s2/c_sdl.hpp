/* Proceduru ‹sdl_frame› doplňte podle komentářů, tento soubor
 * pomocí direktivy ‹#include› vložte do svého ‹c_robots.cpp› a ve
 * své funkci ‹main› zavolejte ‹sdl_run( hra )› (do hry musíte
 * samozřejmě nejprve přidat nějaké roboty). Hru pak přeložte
 * s přepínači ‹-I/usr/include/SDL2 -lSDL2›. */

#include <SDL.h>

const int sdl_width = 1600,
          sdl_height = 900;

void sdl_frame( SDL_Renderer *render, auto &game )
{
    const std::vector< std::tuple< uint8_t, uint8_t, uint8_t > > color
    {
        { 200, 60, 60 },
        { 60, 200, 60 },
        { 60, 60, 200 }
    };

    for ( const auto &bot : /* všichni roboti */ )
    {
        auto [ x, y, z ] = /* souřadnice robota */;
        auto [ cr, cg, cb ] = color[ /* barva robota: 0 = červený, … */ ];

        SDL_Rect box{ .x = int( 10 * x - 3 + sdl_width / 2 ),
                      .y = int( 10 * y - 3 + sdl_height / 2 ),
                      .w = 7, .h = 7 };

        SDL_SetRenderDrawColor( render, cr, cg, cb, SDL_ALPHA_OPAQUE );

        if ( /* vlastník robota je 1 */ )
            SDL_RenderFillRect( render, &box );
        else
            SDL_RenderDrawRect( render, &box );
    }
}

void sdl_error()
{
    std::cerr << "SDL error: " << SDL_GetError();
    std::exit( 1 );
}

void sdl_run( auto &game )
{
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        sdl_error();

    SDL_Window *win = SDL_CreateWindow( "c_robots",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        sdl_width, sdl_height, SDL_WINDOW_SHOWN );

    if ( !win )
        sdl_error();

    SDL_Renderer *render = SDL_CreateRenderer( win, -1, 0 );

    if ( !render )
        sdl_error();

    while ( !SDL_QuitRequested() )
    {
        SDL_SetRenderDrawColor( render, 0, 0, 0, SDL_ALPHA_OPAQUE );
        SDL_RenderClear( render );
        sdl_frame( render, game );
        SDL_RenderPresent( render );
        SDL_Delay( 16 );

        game.tick();
    }
}
