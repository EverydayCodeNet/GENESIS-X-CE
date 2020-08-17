#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

/* Include the converted graphics file */
#include "gfx/gfx.h"

kb_key_t key;

typedef struct {
    int seed;
    int cursorx;
    int cursory;
    int year;
    bool menu;
    bool doCursor;
    int zAvg;
    uint8_t menuSelected;
    int tileSelected;
    int genCoin;
    //id to building that will be used as cursor
    int buildingSelected;
    //number of iron mines, etc?
    int numLoops;
    //water level that will increase with ice to water gens
    uint8_t waterLevel;

    //disable keyboard scan if this is enabled
    bool canPress;
} game_t;
game_t game;

const char *appvarName = "slota";

unsigned int TERRAIN_WIDTH = 24;
unsigned int TERRAIN_HEIGHT = 12;

typedef struct {
    //columns and rows
    int x;
    int y;
    int topX;
    int topY;
    int x2,x3,x4;
    int y2,y3,y4;
    int z;
    bool building;
    int buildingID;

    //building ID guide
    //16xx - power
    //23xx - water
    //13xx - iron mine
} terrain_t;

terrain_t terrainTiles[400];

int xOffset;
int yOffset;
int numTiles = 400;

void SaveData(void) {
    ti_var_t slota;
    ti_CloseAll();
    //if (slota = ti_Open("HUEDAT","w+")) ti_Write(&game,sizeof(game),1,slota);
    if (slota = ti_Open("HUEDAT","w+")) {
        ti_Write(&game,sizeof(game),1,slota);
        ti_Write(&terrainTiles,sizeof(terrainTiles),1,slota);
    }
    
    ti_SetArchiveStatus(true, slota);
    //for (i = 0; i < 7; i++) ti_Write(region[i].data, region[i].spr->width * region[i].spr->height, 1, slota);
    //for (idx = 0; idx < 400; idx++) ti_Write(&terrainTiles,sizeof(terrain_t),1,slota);
    //for (idx = 0; idx < 400; idx++) ti_Write(&terrainTiles, sizeof(terrain_t), 1, slota);
    //ti_SetArchiveStatus(true, slota);
}

void LoadData(void) {
    ti_var_t slota;
    ti_CloseAll();
    if (slota = ti_Open("GENDAT", "r")) {
        ti_Read(&game, sizeof(game),1,slota);
        ti_Read(&terrainTiles, sizeof(terrainTiles),1,slota);
    }
}

void CreateSave(void) {
    ti_var_t slota;
    ti_CloseAll();
    if (slota = ti_Open("GENDAT","w+")) {
        ti_Write(&game, sizeof(game), 1, slota);
        ti_Write(&terrainTiles, sizeof(terrainTiles),1,slota);
        ti_SetArchiveStatus(true, slota);
    }
}

void createTerrain() {
    ti_var_t slota;
    int idx = 0;
    int row;
    int column;
    uint8_t randID = randInt(0,3);
    if (idx < numTiles) {
        for (row = 0; row < 20; row++) {
            for (column = 0; column < 20; column++) {
                terrain_t* terrain = &(terrainTiles[idx]);
                terrain_t prevCol = terrainTiles[idx - 1];
                terrain_t prevRow = terrainTiles[idx - 20];
                terrain->x = column; 
                terrain->y = row;
                terrain->topX = (terrain->x - terrain->y) * (TERRAIN_WIDTH / 2) + 160;
                terrain->topY = (terrain->x + terrain->y) * (TERRAIN_HEIGHT /2) + (240 - 12 * 20) + 10;
                terrain->x2 = terrain->topX + (TERRAIN_WIDTH / 2);
                terrain->y2 = terrain->topY + (TERRAIN_HEIGHT / 2);
                terrain->x3 = terrain->topX;
                terrain->y3 = terrain->topY + TERRAIN_HEIGHT;
                terrain->x4 = terrain->topX - (TERRAIN_WIDTH / 2);
                terrain->y4 = terrain->topY + (TERRAIN_HEIGHT / 2);
                //terrain->z = ti_Read(&terrainTiles[idx].z, sizeof(terrain_t), 1, slota);
                if (idx == 0) {
                    terrain->z = randInt(-15,50);
                } else if (idx < 20 && idx != 0 && column != 0) {
                    terrain->z = prevCol.z + randInt(-5,5);
                } else if (idx > 19 && column != 0) {
                    terrain->z = ((prevCol.z + prevRow.z) / 2) + randInt(-5,5);
                } else if (idx > 19 && column == 0) {
                    terrain->z = prevRow.z + randInt(-5,5);
                }
                
                if (idx == 210) terrain->building = 1; terrain->buildingID = 1301;
                /*if (randInt(0,10) == 1 && terrain->z > 0) {
                    terrain->building = 1;
                    if (idx % 3 == 0) {
                        terrain->buildingID = 1301;
                    } else if (idx % 2 == 0) {
                        terrain->buildingID = 1602;
                    } else if (idx % 5 == 0) {
                        terrain->buildingID = 2301;
                    }
                }*/
                idx++;
            }
        }
    }
}

void drawTiles() {
 //tile borders
    int idx;
    for (idx = 0; idx < numTiles; idx++) {
        terrain_t terrain = terrainTiles[idx];
        gfx_SetColor(255);
        gfx_Line(terrain.topX - xOffset,terrain.topY - yOffset,terrain.x2 - xOffset,terrain.y2 - yOffset);
        gfx_Line(terrain.x2 - xOffset,terrain.y2 - yOffset,terrain.x3 - xOffset,terrain.y3 - yOffset);
        gfx_Line(terrain.x3 - xOffset,terrain.y3 - yOffset,terrain.x4 - xOffset,terrain.y4 - yOffset);
        gfx_Line(terrain.x4 - xOffset,terrain.y4 - yOffset,terrain.topX - xOffset,terrain.topY - yOffset);
    }
}

void drawTerrain() {
    int idx;
    
    for (idx = 0; idx < 400; idx++) {
        terrain_t terrain = terrainTiles[idx];
        terrain_t prev = terrainTiles[idx - 1];
        //terrain_t next = terrainTiles[idx + 1];
        terrain_t nextRow = terrainTiles[idx + 20];
        //draw height
        //back right
        if (terrain.x > 0 && terrain.x < 320 && terrain.y < 240 && terrain.y > 0) {
            
        }
        
        gfx_SetColor(224);
        if (terrain.z < 0) gfx_SetColor(17);
        gfx_FillTriangle(terrain.topX - xOffset,terrain.topY - yOffset,
                    terrain.topX - xOffset,terrain.topY - terrain.z - yOffset,
                    terrain.x2 - xOffset, terrain.y2 - terrain.z - yOffset);
        gfx_FillTriangle(terrain.topX - xOffset,terrain.topY - yOffset,
                        terrain.x2 - xOffset,terrain.y2 - yOffset,
                        terrain.x2 - xOffset, terrain.y2 - terrain.z - yOffset);
        
        //back left - works
        //gfx_SetColor(64); - color shifted due to palette
        gfx_SetColor(32);
        if (terrain.z < 0) gfx_SetColor(17);
        if (prev.z > terrain.z) {
            gfx_FillTriangle(terrain.x4 - xOffset,terrain.y4 - yOffset,
                        terrain.x4 - xOffset,terrain.y4-terrain.z - yOffset,
                        terrain.topX - xOffset,terrain.topY - yOffset);
            gfx_FillTriangle(terrain.topX - xOffset,terrain.topY -terrain.z - yOffset,
                            terrain.topX - xOffset,terrain.topY - yOffset,
                            terrain.x4 - xOffset,terrain.y4-terrain.z - yOffset);
        }   

        gfx_SetColor(32);
        if (terrain.z < 0) gfx_SetColor(17);
        //front left
        //if next row is deeper than row, don't display front/left
        //last row should always show face
        if (terrain.z > nextRow.z || idx > 379) {
            gfx_FillTriangle(terrain.x4 - xOffset,terrain.y4 - yOffset,
            terrain.x4- xOffset,terrain.y4 - yOffset - terrain.z,
            terrain.x3- xOffset,terrain.y3 - yOffset - terrain.z);
            gfx_FillTriangle(terrain.x4- xOffset,terrain.y4 - yOffset,
            terrain.x3- xOffset,terrain.y3 - yOffset,
            terrain.x3- xOffset,terrain.y3 - yOffset - terrain.z);
        }

        //front right
        gfx_SetColor(96);
        //color keeps getting shifted because of global palette?
        //gfx_SetColor(64);
        if (terrain.z < 0) gfx_SetColor(17);
        
        gfx_FillTriangle(terrain.x3 - xOffset,terrain.y3 - yOffset,
                        terrain.x3 - xOffset,terrain.y3 - yOffset - terrain.z, 
                        terrain.x2 - xOffset,terrain.y2 - yOffset - terrain.z);
        gfx_FillTriangle(terrain.x3 - xOffset,terrain.y3 - yOffset,
                        terrain.x2 - xOffset,terrain.y2 - yOffset, 
                        terrain.x2 - xOffset,terrain.y2 - yOffset - terrain.z);
        //top
        gfx_SetColor(224);
        if (terrain.z < 0) gfx_SetColor(17);
        //snow
        if (terrain.z > 25) gfx_SetColor(255);
        gfx_FillTriangle(terrain.x4 - xOffset, terrain.y4 - yOffset - terrain.z,
                        terrain.topX - xOffset, terrain.topY - yOffset - terrain.z,
                        terrain.x3 - xOffset, terrain.y3 - yOffset - terrain.z);

        gfx_FillTriangle(terrain.x3 - xOffset,terrain.y3 - terrain.z - yOffset,
                        terrain.topX - xOffset, terrain.topY - yOffset - terrain.z,
                        terrain.x2 - xOffset, terrain.y2 - yOffset - terrain.z);
    }
}

void drawBuildings() {
    int idx;
    for (idx = 0; idx < 400; idx++) {
        terrain_t terrain = terrainTiles[idx];
        if (terrain.building == 1) {
            //set colors
            gfx_SetPalette(global_palette, sizeof_global_palette, 0);
            gfx_SetTransparentColor(0);
            //gfx_TransparentSprite(house_scaled, tile.topX - xOffset - 25, tile.y3 - yOffset - 41);
            //width / 2, height
            if (terrain.buildingID == 1601) gfx_TransparentSprite(solar_panel, terrain.topX - xOffset - 12, terrain.y3 - yOffset - 26);
            if (terrain.buildingID == 1602) gfx_TransparentSprite(power_plant, terrain.topX - xOffset - 12, terrain.y3 - yOffset - 26);
            if (terrain.buildingID == 2301) gfx_TransparentSprite(water, terrain.topX - xOffset - 12, terrain.y3 - yOffset - 26);
            if (terrain.buildingID == 1301) gfx_TransparentSprite(iron_mine, terrain.topX - xOffset - 12, terrain.y3 - yOffset - 26);
        }
    }
}

void WhiText(void) {
    gfx_SetTextBGColor(0);
    gfx_SetTextFGColor(255);
    gfx_SetTextTransparentColor(0);
}

void YelText(void) {
    gfx_SetTextBGColor(0);
    gfx_SetTextFGColor(229);
    gfx_SetTextTransparentColor(0);
}

void BlaText(void) {
    gfx_SetTextBGColor(255);
    gfx_SetTextFGColor(0);
    gfx_SetTextTransparentColor(255);
}

void mainMenu() {
    int idx;
    uint8_t menuSelected;
    do {
        kb_Scan();
        gfx_SetDrawBuffer();

        gfx_FillScreen(0);
        //draw stars
        WhiText();
        gfx_SetTextScale(2,2);
        gfx_PrintStringXY("START GAME",(160 - gfx_GetStringWidth("START GAME") / 2),58);
        gfx_PrintStringXY("HOW TO PLAY",(160 - gfx_GetStringWidth("HOW TO PLAY") / 2),94);
        gfx_PrintStringXY("ABOUT",(160 - gfx_GetStringWidth("ABOUT") / 2),130);
        gfx_PrintStringXY("QUIT",(160 - gfx_GetStringWidth("QUIT") / 2),166);
        
        if (kb_Data[7] == kb_Up && menuSelected != 0) {
            delay(150);
            menuSelected--;
        } else if (kb_Data[7] == kb_Down && menuSelected != 3) {
            delay(150);
            menuSelected++;
        }

        if (menuSelected == 0) {
            YelText();
            gfx_PrintStringXY("START GAME",(160 - gfx_GetStringWidth("START GAME") / 2),58);
        } else if (menuSelected == 1) {
            YelText();
            gfx_PrintStringXY("HOW TO PLAY",(160 - gfx_GetStringWidth("HOW TO PLAY") / 2),94);
        } else if (menuSelected == 2) {
            YelText();
            gfx_PrintStringXY("ABOUT",(160 - gfx_GetStringWidth("ABOUT") / 2),130);
        } else if (menuSelected == 3) {
            YelText();
            gfx_PrintStringXY("QUIT",(160 - gfx_GetStringWidth("QUIT") / 2),166);
        }
        gfx_SwapDraw();
    } while(kb_Data[6] != kb_Enter);   
}

//used when loading map after exiting tab
void progressBar() {
    int idx;
    //take idx from draw terrain
    //drawn from top left corner
    WhiText();
    //... should be animated using modulo command
    gfx_PrintStringXY("Loading",40,32);
    gfx_SetColor(0);
    //height of each 
    gfx_FillRectangle(40,110,240,20);
    for (idx = 0; idx < numTiles; idx++) {
        terrain_t terrain = terrainTiles[idx];
        //setdraw buffer then swapdraw
        //height of each
        gfx_SetColor(224); 
        gfx_SetDrawBuffer();
        gfx_HorizLine(idx + 40, 130 - (terrain.z / 10), terrain.z / 10);
        //gfx_HorizLine(idx + 40, 130 - terrain.z, terrain.z);
        gfx_SwapDraw();
    }
}

void drawTabs() {
    gfx_SetColor(0);
    gfx_FillRectangle(0,0,320,30);
    gfx_FillRectangle(0,210,320,30);
    //white lines
    gfx_SetColor(255);
    gfx_HorizLine(0,210,320);
    gfx_VertLine(100,210,30);
    gfx_VertLine(220,210,30);
    gfx_HorizLine(0,30,320);
    WhiText();
    gfx_SetTextScale(1,1);
    if (game.menuSelected == 0) {
        gfx_PrintStringXY("MARS",10,12);
        gfx_PrintStringXY("YEAR 202",310 - gfx_GetStringWidth("YEAR 2020"),12);
        gfx_PrintInt(game.year,1);
        WhiText();
        gfx_PrintStringXY("BUILD",50 - gfx_GetStringWidth("BUILD") / 2,222);
        gfx_PrintStringXY("STATS",160 - gfx_GetStringWidth("STATS") / 2,222);
        gfx_PrintStringXY("EXIT",270 - gfx_GetStringWidth("EXIT") / 2,222);

    } else if (game.menuSelected == 1) {
        //gold 
        //population
        gfx_SetColor(229);
        gfx_FillCircle(11,14,8);
        YelText();
        gfx_PrintStringXY("",25,12);
        gfx_PrintInt(game.genCoin,1);
        WhiText();
        gfx_PrintStringXY("YEAR 202",310 - gfx_GetStringWidth("YEAR 2020"),12);
        gfx_PrintInt(game.year,1);
    }
    if (game.menuSelected == 1) {
        YelText();
        gfx_PrintStringXY("BUILD",50 - gfx_GetStringWidth("BUILD") / 2,222);
        WhiText();
        gfx_PrintStringXY("STATS",160 - gfx_GetStringWidth("STATS") / 2,222);
        gfx_PrintStringXY("EXIT",270 - gfx_GetStringWidth("EXIT") / 2,222);
    } else if (game.menuSelected == 2) {
        WhiText();
        gfx_PrintStringXY("BUILD",50 - gfx_GetStringWidth("BUILD") / 2,222);
        YelText();
        gfx_PrintStringXY("STATS",160 - gfx_GetStringWidth("STATS") / 2,222);
        WhiText();
        gfx_PrintStringXY("EXIT",270 - gfx_GetStringWidth("EXIT") / 2,222);
    } else if (game.menuSelected == 3) {
        WhiText();
        gfx_PrintStringXY("BUILD",50 - gfx_GetStringWidth("BUILD") / 2,222);
        gfx_PrintStringXY("STATS",160 - gfx_GetStringWidth("STATS") / 2,222);
        YelText();
        gfx_PrintStringXY("EXIT",270 - gfx_GetStringWidth("EXIT") / 2,222);
    }
}

void drawCursor() {
    int idx;
    terrain_t* terrain = &(terrainTiles[game.tileSelected]);
    gfx_SetColor(255);
    //change cursor to building selected
    if (game.buildingSelected == 0) {
        gfx_Circle(game.cursorx,game.cursory,5);
        gfx_SetColor(0);
        gfx_SetPixel(game.cursorx,game.cursory);
        gfx_SetPixel(game.cursorx - 1,game.cursory);
        gfx_SetPixel(game.cursorx + 1,game.cursory);
        gfx_SetPixel(game.cursorx,game.cursory - 1);
        gfx_SetPixel(game.cursorx,game.cursory + 1);
    } else {
        /*if (game.buildingSelected == 1301) {
            gfx_TransparentSprite(iron_mine, game.cursorx, game.cursory);
        } else if (game.buildingSelected == 1601) {
            gfx_TransparentSprite(solar_panel, game.cursorx, game.cursory);
        } else if (game.buildingSelected == 1602) {
            gfx_TransparentSprite(power_plant, game.cursorx, game.cursory);
        } else if (game.buildingSelected == 2301) {
            gfx_TransparentSprite(water, game.cursorx, game.cursory);
        }*/

        if (kb_Data[7] == kb_Down && game.tileSelected < 379) {
            delay(150);
            game.tileSelected = game.tileSelected + 20; 
        } else if (kb_Data[7] == kb_Up && game.tileSelected > 19) {
            delay(150);
            game.tileSelected = game.tileSelected - 20;
        } else if (kb_Data[7] == kb_Left && game.tileSelected != 0) {
            delay(150);
            game.tileSelected--;
        } else if (kb_Data[7] == kb_Right && game.tileSelected != 379)  {
            delay(150);
            game.tileSelected++;
        }

        if (game.buildingSelected == 1301) gfx_TransparentSprite(iron_mine, terrain->topX - 12, terrain->y3 - 26);
        if (game.buildingSelected == 1601) gfx_TransparentSprite(solar_panel, terrain->topX - 12, terrain->y3 - 26);
        if (game.buildingSelected == 1602) gfx_TransparentSprite(power_plant, terrain->topX - 12, terrain->y3 - 26);
        if (game.buildingSelected == 2301) gfx_TransparentSprite(water,terrain->topX - 12, terrain->y3 - 26);

        gfx_SetColor(255);
        gfx_Line(terrain->topX - xOffset,terrain->topY - yOffset,terrain->x2 - xOffset,terrain->y2 - yOffset);
        gfx_Line(terrain->x2 - xOffset,terrain->y2 - yOffset,terrain->x3 - xOffset,terrain->y3 - yOffset);
        gfx_Line(terrain->x3 - xOffset,terrain->y3 - yOffset,terrain->x4 - xOffset,terrain->y4 - yOffset);
        gfx_Line(terrain->x4 - xOffset,terrain->y4 - yOffset,terrain->topX - xOffset,terrain->topY - yOffset);

        if (kb_Data[6] == kb_Enter && terrain->z > game.waterLevel && terrain->building == false) {
            BlaText();
            gfx_SetTextScale(2,2);
            gfx_PrintStringXY("THIS IS WORKING", 160 - (gfx_GetStringWidth("THIS IS WORKING") / 2), (240 - 8) / 2);
            gfx_SetTextScale(1,1);
            terrain->building = true;
            terrain->buildingID = game.buildingSelected;
            game.buildingSelected = 0;
            //update map
            gfx_SetDrawBuffer();
            drawBuildings();
            gfx_SwapDraw();
        } else if (terrain->z <= game.waterLevel) {
            BlaText();
            gfx_SetTextScale(2,2);
            gfx_PrintStringXY("Cannot build on water.", 160 - (gfx_GetStringWidth("Cannot build on water.") / 2), (240 - 8) / 2);
            gfx_SetTextScale(1,1);
        } else if (terrain->building == true) {
            BlaText();
            gfx_SetTextScale(2,2);
            gfx_PrintStringXY("Tile is occupied.", 160 - (gfx_GetStringWidth("Tile is occupied.") / 2), (240 - 8) / 2);
            gfx_SetTextScale(1,1);
        }
        //turn off cursor
        
        
        //place down building
        /*if (kb_Data[6] == kb_Add) {
            //get cursor x and find the tile that the building belongs
            //check if tile is not water
            //maybe draw outline instead
            for (idx = 0; idx < numTiles; idx++) {
                terrain_t* terrain = &(terrainTiles[idx]);
                if (game.cursorx + 5 < terrain->x2 && game.cursorx - 5 > terrain->x4 && game.cursory + 5 < terrain->y3 && game.cursory - 5 > terrain->topX) {
                    //if tile is not occupied and not water
                    if (terrain->z > 0) {
                        if (terrain->building == false) {
                            terrain->building = true;
                            terrain->buildingID = game.buildingSelected;
                            game.buildingSelected = 0;
                            //update map
                            //set it to draw buffer?
                            gfx_SetDrawBuffer();
                            drawBuildings();
                            gfx_SwapDraw();
                            //gfx_SetDrawScreen();
                        }
                    } else if (terrain->z < 0) {
                        BlaText();
                        gfx_SetTextScale(2,2);
                        gfx_PrintStringXY("Cannot build on water.", 160 - (gfx_GetStringWidth("Cannot build on water.") / 2), (240 - 8) / 2);
                        gfx_SetTextScale(1,1);
                    }
                    
                }
            }
        }*/
    }
}

void shiftMap() {
    gfx_SetDraw(1);
    gfx_SetDrawBuffer();
    gfx_FillScreen(0);
    drawTerrain();
    drawBuildings();
    drawTabs();
    gfx_SetDrawScreen();
    drawCursor();
    //gfx_SwapDraw();
}

void dispStats() {
    terrain_t terrain = terrainTiles[game.tileSelected];
    WhiText();
    gfx_PrintStringXY("Index",10,50);
    gfx_PrintInt(game.tileSelected,1);
    gfx_PrintStringXY("Building",10,60);
    gfx_PrintInt(terrain.building,1);
}

void doRedraw() {
    //act on the screen
    gfx_SetDrawScreen();
    //gfx_FillScreen(0);
    //gfx_SetColor(0);
    //gfx_Circle(game.cursorx,game.cursory,5);
    //redraw buffered background
    gfx_Blit(1);
    //temporary clear screen for stats
    
    drawCursor();
    //dispStats();
}

void drawScreen() {
    uint8_t buildingSelected = 0;
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(0);
    //gfx_SetDrawScreen();
    gfx_FillScreen(0);
    if (game.menuSelected == 1) {
        //build menu
        drawTabs();
        key = kb_Data[6];
        gfx_Begin();
        //gfx_SetDrawBuffer();
        
        gfx_SetPalette(global_palette, sizeof_global_palette, 0);
        gfx_SetTransparentColor(0);
        do {
            kb_Scan();
            gfx_SetDrawBuffer();
            gfx_FillScreen(0);
            drawTabs();
            if (kb_Data[7] == kb_Up && buildingSelected != 0) {
                delay(150);
                buildingSelected--;
            } else if (kb_Data[7] == kb_Down && buildingSelected != 3) {
                delay(150);
                buildingSelected++;
            }
            game.numLoops++;
            game.genCoin = game.numLoops / 10;
            gfx_TransparentSprite(solar_panel, 10, 40);
            gfx_TransparentSprite(power_plant, 10, 70);
            gfx_TransparentSprite(water, 10, 100);
            gfx_TransparentSprite(iron_mine, 10, 130);
            //change cursor to building
            WhiText();
            gfx_PrintStringXY("Solar Panel", 40, 50);
            gfx_PrintStringXY("Coal Power Plant", 40, 80);
            gfx_PrintStringXY("Ice to Water Generator", 40, 110);
            gfx_PrintStringXY("Iron Mine", 40, 140);
            if (buildingSelected == 0) {
                game.buildingSelected = 1601;
                YelText();
                gfx_PrintStringXY("Solar Panel", 40, 50);
                WhiText();
            } else if (buildingSelected == 1) {
                game.buildingSelected = 1602;
                YelText();
                gfx_PrintStringXY("Coal Power Plant", 40, 80);
                WhiText();
            } else if (buildingSelected == 2) {
                game.buildingSelected = 2301;
                YelText();
                gfx_PrintStringXY("Ice to Water Generator", 40, 110);
                WhiText();
            } else if (buildingSelected == 3) {
                game.buildingSelected = 1301;
                YelText();
                gfx_PrintStringXY("Iron Mine", 40, 140);
            }
            gfx_SwapDraw();
        } while(kb_Data[6] != kb_Enter);
        
        game.menuSelected = 0;
        
        gfx_SetDrawScreen();
        gfx_FillScreen(0);
        
        drawTerrain();
        //place inside of draw terrain function
        //progressBar();
        drawBuildings();
        
        drawTabs();
        gfx_Blit(0);
        gfx_SwapDraw();
        gfx_SetDrawScreen();
        drawCursor();
    } else if (game.menuSelected == 2) {
        //statistics
        drawTabs();
    } else if (game.menuSelected == 3) {
        //exit
    }
}

void handleKeys() {
    uint8_t buildingSelected = 0;
    key = kb_Data[1];

    if (key == kb_Yequ) {
        //build screen
        game.menuSelected = 1;
        drawScreen();
        
    } else if (key == kb_Zoom) {
        //stats- probably change to zoom in, have stats be third tab
        game.menuSelected = 2;
        drawScreen();
    } else if (key == kb_Graph || kb_Data[6] == kb_Clear) {
        game.menuSelected = 3;
        //game.tileSelected++;
        //save game? 
    }
}

void doCursor() {
    key = kb_Data[7];
    //erase cursor
    if (key == kb_Up) {
        game.cursory--;
        doRedraw();
    } else if (key == kb_Down) {
        game.cursory++;
        doRedraw();
    } else if (key == kb_Left) {
        game.cursorx--;
        doRedraw();
    } else if (key == kb_Right) {
        game.cursorx++;
        doRedraw();
    }

    if (game.cursory - 5 < 29) {
        yOffset = yOffset - 20;
        shiftMap();
        game.cursory = 50;
        doRedraw();
    } else if (game.cursory + 5 > 201) {
        yOffset = yOffset + 20;
        shiftMap();
        game.cursory = 190;
        doRedraw();
    } else if (game.cursorx - 5 < -5) {
        xOffset = xOffset - 20;
        shiftMap();
        game.cursorx = 20;
        doRedraw();
    } else if (game.cursorx > 315) {
        xOffset = xOffset + 20;
        shiftMap();
        game.cursorx = 300;
        doRedraw();
    }
}

void delBuildings() {
    int idx;
    for (idx = 0; idx < numTiles; idx++) {
        terrain_t* terrain = &(terrainTiles[idx]);
        terrain->building = false;
    }
    game.buildingSelected = 0;
}

void startGame() {
    srand(rtc_Time());
    game.seed = rtc_Time();
    createTerrain();
    game.cursorx = 160;
    game.cursory = 120;
    game.doCursor = true;
    gfx_SetDrawScreen();
    gfx_FillScreen(0);
    drawTerrain();
    drawBuildings();
    drawTabs();
    gfx_Blit(0);
    gfx_SwapDraw();
    gfx_SetDrawScreen();
    drawCursor();
}

void main() {
    gfx_Begin();
    mainMenu();
    
    gfx_Begin();
    LoadData();
    game.seed = rtc_Time();
    srand(rtc_Time());
    createTerrain();
    //may cause issue with placing buildings
    game.cursorx = 160;
    game.cursory = 120;
    game.doCursor = true;
    game.canPress = true;
    gfx_SetDrawScreen();
    gfx_FillScreen(0);
    drawTerrain();
    drawBuildings();
    
    drawTabs();
    gfx_Blit(0);
    gfx_SwapDraw();
    gfx_SetDrawScreen();
    drawCursor();
    
    do {
        kb_Scan();
        doCursor();
        handleKeys();
        //update gold with every iron mine
        game.numLoops++;
        game.genCoin = game.numLoops / 10;
        if (kb_Data[1] == kb_Del) startGame();
    } while (kb_Data[6] != kb_Clear);
    CreateSave();
    SaveData();
    gfx_End();
}