#include <allegro.h>
#include <allegro/keyboard.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 600
#define BULLETS 10
#define NOP 4

BITMAP *pages [NOP]; //number of pages
BITMAP *glowna;
BITMAP *menu1;
BITMAP *menu2;
BITMAP *history;
BITMAP *tlo;
BITMAP *ship_texture;
BITMAP *bullet_texture;
BITMAP *alien_bullet_texture;
//SAMPLE *dzwiek;

char active_map[] = "mapa1.txt";
int toReload;

struct enemy_node
{
    int life;
    int x,y;
    BITMAP *enemy_texture;
};
struct bullet
{
    int x,y;
    int dmg;
}bullets[BULLETS], alien_bullets[BULLETS];
struct ship_data
{
    int x,y;
    BITMAP *body;
};
struct ship_data ship ={WIDTH/2, HEIGHT-75,NULL};

int num_of_bullets;
int num_of_alien_bullets;


struct enemy_node** aliens;
int check_win(int x, int y)
{
    int i,j;
    for (i=0;i<x;i++)
        for (j=0;j<y;j++)
            if (aliens[i][j].life > 0)
                return 1;
    return 0;
}
void create_aliens(int x, int y, char* filename)
{
    int i = 0,j = 0;
    if (filename != NULL){
        char znak = 0;
        int tmp_life = 0;
        FILE* plik = fopen(filename, "r");
        fscanf(plik, "%d", &tmp_life);
        fscanf(plik, "%d", &tmp_life);
        while(!feof(plik))
        {
            znak = fgetc(plik);
            if (znak == EOF) break;
            if (znak == '\n') continue;
            if (j == y) {i++; j=0;}
            if (i == x) break;
            if (znak == 'a')
                aliens[i][j].life = 2;
            if (znak == 'b')
                aliens[i][j].life = 0;
            aliens[i][j].x = i*80 + 30;
            aliens[i][j].y = j*70 + 8;
            aliens[i][j].enemy_texture = load_bmp("Image\\invander.bmp",default_palette);
            j++;
        }
        fclose(plik);
    }
}

void draw_aliens(BITMAP *active_page, int x, int y)
{
    int i,j;
    for(i=0;i<x;i++)
        for(j=0; j<y; j++)
        {
            if(aliens [i][j].life > 0)
                masked_blit(aliens[i][j].enemy_texture, active_page, 0, 0, aliens[i][j].y, aliens[i][j].x, aliens[i][j].enemy_texture->w, aliens[i][j].enemy_texture->h);
        }
}

void shot()
{
    if(key[KEY_LCONTROL] && num_of_bullets<1)
    {
        num_of_bullets++;
        bullets[num_of_bullets-1].x = ship.x+17;
        bullets[num_of_bullets-1].y = ship.y;
        bullets[num_of_bullets-1].dmg = 1;
    }
}

int find_free_bullet()
{
    int i;
    for (i=1;i<BULLETS;i++)
        if (alien_bullets[i].dmg == 0)
            return i;
    return 1;
}

void alien_shot(int x, int y)
{
    if(num_of_alien_bullets<1)
    {
        num_of_alien_bullets++;
        alien_bullets[find_free_bullet()-1].dmg = 1;
        alien_bullets[find_free_bullet()-1].x = x+20;
        alien_bullets[find_free_bullet()-1].y = y;
    }
}

void alien_shot_central(int x, int y)
{
    int i,j;
    for (i=0;i<x;i++)
        for (j=0;j<y;j++){
            if (rand()%255 == 0 && aliens[i][j].life > 0)
                alien_shot(aliens[i][j].y, aliens[i][j].x);
        }
}

void draw_bullets(BITMAP *active_page)
{
    int i;
    for (i=0;i<num_of_bullets;i++)
    {
        if (bullets[i].dmg)
            masked_blit(bullet_texture, active_page, 0, 0, bullets[i].x, bullets[i].y, bullet_texture->w, bullet_texture->h);
    }
}

void draw_alien_bullets(BITMAP* active_page)
{
    int i;
    for (i=0;i<BULLETS;i++)
    {
        if (alien_bullets[i].dmg > 0)
            masked_blit(alien_bullet_texture, active_page, 0, 0, alien_bullets[i].x, alien_bullets[i].y, alien_bullet_texture->w, alien_bullet_texture->h);
    }
}

void update_bullets()
{
    int i;
    for (i=0;i<num_of_bullets;i++)
    {
        bullets[i].y -=30;
        if (bullets[i].y<0)
        {
            int j;
            for (j=num_of_bullets-1;j>=i;j--)
            {
                bullets[j] = bullets[j-1];
            }
            num_of_bullets--;
        }
    }
}
void update_alien_bullets()
{
    int i;
    for (i=0;i<num_of_alien_bullets;i++)
    {
        if (alien_bullets[i].dmg > 0)
            alien_bullets[i].y +=7;
        if (alien_bullets[i].y>600)
        {
            alien_bullets[i].dmg = 0;
            alien_bullets[i].x = 0;
            alien_bullets[i].y = 0;
            num_of_alien_bullets--;
        }
    }
}

void moves_ship()
{
    if(key[KEY_LEFT] && ship.x>0) ship.x-=4;
    if(key[KEY_RIGHT] && ship.x<(WIDTH-ship_texture->w)) ship.x+=4;
}


int create_pages_array(BITMAP *pages [NOP])
{
    int i;
    for(i=0; i<NOP; i++)
    {
        pages[i] = create_video_bitmap(SCREEN_W,SCREEN_H);
        if(pages[i]==NULL)
            return 0;
    }
    return 1;
}

void destroy_pages_array(BITMAP *pages[NOP])
{
    int i;
    for(i=0; i<NOP; i++)
        destroy_bitmap(pages[i]);
}

void draw_ship(BITMAP *active_page)
{
    masked_blit(ship_texture, active_page, 0, 0, ship.x, ship.y, ship_texture->w, ship_texture->h);
}

///KOLIZJE
void collisions(struct bullet bullets[BULLETS], struct enemy_node** aliens, int x, int y)
{
    int i;
    for (i=0;i<BULLETS;i++){
        int j,k;
        for (j=x-1;j>=0;j--)
            for (k=0;k<y;k++)
            {
                if (bullets[i].dmg && aliens[j][k].life > 0)
                    if (aliens[j][k].y < bullets[i].x && bullets[i].x < aliens[j][k].y+47)
                    {
                        if (j*70+91 > bullets[i].y && bullets[i].y > j*70+61){
                            aliens[j][k].life -= bullets[i].dmg;
                            bullets[i].dmg = 0;
                            toReload = !check_win(x,y);
                        }
                    }
            }
    }
}

int collisions_ship()
{
    int i;
    for (i=0;i<BULLETS;i++){
        if (alien_bullets[i].x > ship.x && alien_bullets[i].x < ship.x+35)
            if (alien_bullets[i].y > ship.y && alien_bullets[i].y < ship.y+25)
                return 1;
    }
    return 0;
}
void allocate_aliens(int *x, int *y, char* filename)
{
    FILE* wsk;
    wsk = fopen(filename, "r");
    fscanf(wsk, "%d", x);
    fscanf(wsk, "%d", y);
    fclose(wsk);
    aliens = (struct enemy_node**)calloc(*x, sizeof(struct enemy_node*));
    int i;
    for (i=0;i<*x;i++)
        aliens[i] = (struct enemy_node*)calloc(*y, sizeof(struct enemy_node));
}
void deallocate_aliens(int x, int y)
{
    int i;
    for (i=0;i<x;i++)
        free(aliens[i]);
    free(aliens);
}
void move_aliens(int x, int y, int shift)
{
    int i,j;
    for (i=0;i<x;i++)
        for (j=0;j<y;j++)
            aliens[i][j].y += shift;
}

void main_animate(int x, int y)
{
    int direction = 1;
    int shift = 0;
    int i=0;
    int fps = 0;
    int fps_to_show = fps;
    long int fps_counter = time(0);

///MENU
    while(!key[KEY_ENTER])
    {
        BITMAP *active_page = pages[i];
        blit(glowna, active_page, 0, 0, 0, 0, glowna->w, glowna->h);
    }

    while(!key[KEY_1])
    {
        BITMAP *active_page = pages[i];
        blit(menu1, active_page, 0, 0, 0, 0, menu1->w, menu1->h);
    }

    while(!key[KEY_SPACE])
    {
        BITMAP *active_page = pages[i];
        blit(history, active_page, 0, 0, 0, 0, history->w, history->h);
    }

     while(!key[KEY_ENTER])
    {
        BITMAP *active_page = pages[i];
        blit(menu2, active_page, 0, 0, 0, 0, menu2->w, menu2->h);
    }
    clear_keybuf();

///MUZYKA
    //play_sample(dzwiek,255,127,1000,1);
    while(!key[KEY_ESC])
    {
        BITMAP *active_page = pages[i];
        moves_ship();
        blit(tlo, active_page, 0, 0, 0, 0, tlo->w, tlo->h);
        draw_ship(active_page);
        draw_aliens(active_page, x, y);
        if (direction > 0)
            move_aliens(x,y,1);
        else
            move_aliens(x,y,-1);
        if (shift >= 100  && i == 1){
            direction = !direction;
            shift = 0;
        }
        shift++;
        draw_alien_bullets(active_page);

        collisions(bullets, aliens, x, y);
        if (toReload)
        {
            active_map[4]++;
            deallocate_aliens(x,y);
            allocate_aliens(&x, &y, active_map);
            create_aliens(x,y, active_map);
            toReload = 0;
        }

        if (collisions_ship()) break;
        alien_shot_central(x,y);
        update_alien_bullets();
        update_bullets();
        shot();

        fps++;
        if (fps_counter != time(0))
        {
            fps_counter = time(0);
            fps_to_show = fps;
            fps = 0;
        }
        textprintf_centre_ex(active_page, font, 750, 10, makecol(255,255,255), makeacol(0,0,0,255), "FPS: %d", fps_to_show);

        draw_bullets(active_page);
/*        if( key[ KEY_1 ] ) { adjust_sample( dzwiek, 255, 127, 1000, 1 ); }
        if( key[ KEY_2 ] ) { adjust_sample( dzwiek, 255, 127, 1500, 1 ); }
        if( key[ KEY_3 ] ) { adjust_sample( dzwiek, 255, 127, 2000, 1 ); }
        if( key[ KEY_4 ] ) { adjust_sample( dzwiek, 255, 0, 1000, 1 ); }
        if( key[ KEY_5 ] ) { adjust_sample( dzwiek, 0, 0, 1000, 1 ); }
*/
        show_video_bitmap(active_page);

        if(i==NOP-1) i=0;
        else i++;
    }

}


int initialize(int card)
{
    if(allegro_init())
    {
        allegro_message("allegro_init(): %s\n",allegro_error);
        allegro_exit();
        return -1;
    }
    if(install_keyboard())
    {
        allegro_message("install_keyboard(): %s\n",allegro_error);
        allegro_exit();
        return -1;
    }
    set_color_depth(32);
    if(set_gfx_mode(card,WIDTH,HEIGHT,0,0))
    {
        allegro_message("set_gfx_mode(): %s\n",allegro_error);
        allegro_exit();
        return -1;
    }
    /*
    install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,"" );
    set_volume(255,255);
    dzwiek =load_sample("muzyka.wav");
    if(!dzwiek)
    {
        set_gfx_mode(GFX_TEXT,0,0,0,0);
        allegro_message("Nie moge zaladowac dzwieku !");
        allegro_exit();
    }*/
    glowna = load_bmp("Image\\glowna.bmp", default_palette);
    menu1= load_bmp("Image\\menu1.bmp", default_palette);
    history = load_bmp("Image\\history.bmp", default_palette);
    menu2= load_bmp("Image\\menu2.bmp", default_palette);
    tlo = load_bmp("Image\\tlo.bmp", default_palette);
    ship_texture = load_bmp("Image\\ship.bmp", default_palette);
    bullet_texture = load_bmp("Image\\bullet.bmp", default_palette);
    alien_bullet_texture = load_bmp("Image\\alien_bullet.bmp", default_palette);
    return 0;
}

int main()
{
    initialize(GFX_AUTODETECT_WINDOWED);
    if(!create_pages_array(pages))
        return 1;

    int x;
    int y;
    blit(tlo, screen, 0, 0, 0, 0, tlo->w, tlo->h);
    allocate_aliens(&x,&y, "mapa1.txt");
    create_aliens(x,y, "mapa1.txt");
    main_animate(x,y);
    destroy_pages_array(pages);
//    stop_sample( dzwiek );
//    destroy_sample( dzwiek );
    allegro_exit();
    return 0;
}
END_OF_MAIN()
