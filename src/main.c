#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

/* Include the converted graphics file */
#include "gfx/gfx.h"

kb_key_t key;

void drawCursor();

typedef struct {
    bool startGame;
    int seed;
    int cursorx;
    int cursory;
    int year;
    bool menu;
    bool doCursor;
    int zAvg;
    bool canExit;
    bool dispStats;

    //could be set to extern int?
    uint8_t menuSelected;
    int tileSelected;
    int genCoin;
    //id to building that will be used as cursor
    int buildingSelected;
    //number of iron mines, etc?
    int numLoops;
    //water level that will increase with ice to water gens
    //water level maybe needs to be int because uint8 will go from 0 to 255
    //waterlevel shouldn't be default, should display snow until planet is hot enough
    uint8_t waterLevel;
    //snow level will decrease with ice to water gens
    uint8_t snowLevel;
    int climate;
    //ice level needed too? if run out of ice, ice to water gens can't work.
    uint8_t evapMultiplier;
    uint8_t waterMultiplier;
    int income;
    //need heat, water, power
    int population;
    //disable keyboard scan if this is enabled
    bool canPress;

    //used for start menu - if no save, new game.
    bool save;
} game_t;
game_t game;

const char *appvarName = "slota";

uint8_t TERRAIN_WIDTH = 24;
uint8_t TERRAIN_HEIGHT = 12;

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
    unsigned int buildingID;

    //building ID guide
    //16xx - power
    //23xx - water
    //13xx - iron mine
    //8xx - habitat
} terrain_t;
terrain_t terrainTiles[400];

int xOffset;
int yOffset;
int numTiles = 400;
bool startGame = false;

void SaveData(void) {
    ti_var_t slota;
    ti_CloseAll();
    if ((slota = ti_Open("GENDAT","w+"))) {
        ti_Write(&game,sizeof(game),1,slota);
        ti_Write(&terrainTiles,sizeof(terrainTiles),1,slota);
    }
    
    ti_SetArchiveStatus(true, slota);
}

void LoadData(void) {
    ti_var_t slota;
    ti_CloseAll();
    if ((slota = ti_Open("GENDAT", "r"))) {
        ti_Read(&game, sizeof(game),1,slota);
        ti_Read(&terrainTiles, sizeof(terrainTiles),1,slota);
    }
}

void CreateSave(void) {
    ti_var_t slota;
    ti_CloseAll();
    if ((slota = ti_Open("GENDAT","w+"))) {
        ti_Write(&game, sizeof(game), 1, slota);
        ti_Write(&terrainTiles, sizeof(terrainTiles),1,slota);
        ti_SetArchiveStatus(true, slota);
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

void createTerrain() {
    int idx = 0;
    uint8_t column, row;

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
                //temporary building test
                //if (idx == 210) terrain->building = 1; terrain->buildingID = 1301;
                idx++;
            }
        }
    }
}

/*void progressBar(int idx) {
    gfx_SetColor(255);
    WhiText();
    gfx_PrintStringXY("Loading...",110,100);
    gfx_FillRectangle(110,110,idx/4,20);
}*/

void drawTerrain() {
    int idx;
    
    for (idx = 0; idx < 400; idx++) {
        terrain_t terrain = terrainTiles[idx];
        terrain_t prev = terrainTiles[idx - 1];
        //terrain_t next = terrainTiles[idx + 1];
        terrain_t nextRow = terrainTiles[idx + 20];
        //draw height

        //back right
        gfx_SetColor(96);
        if (terrain.z < game.waterLevel) gfx_SetColor(16);
        gfx_FillTriangle(terrain.topX - xOffset,terrain.topY - yOffset,
                    terrain.topX - xOffset,terrain.topY - terrain.z - yOffset,
                    terrain.x2 - xOffset, terrain.y2 - terrain.z - yOffset);
        gfx_FillTriangle(terrain.topX - xOffset,terrain.topY - yOffset,
                        terrain.x2 - xOffset,terrain.y2 - yOffset,
                        terrain.x2 - xOffset, terrain.y2 - terrain.z - yOffset);
        
        //back left - works
        //gfx_SetColor(64); - color shifted due to palette
        gfx_SetColor(32);
        if (terrain.z < game.waterLevel) gfx_SetColor(16);
        if (prev.z > terrain.z) {
            gfx_FillTriangle(terrain.x4 - xOffset,terrain.y4 - yOffset,
                        terrain.x4 - xOffset,terrain.y4-terrain.z - yOffset,
                        terrain.topX - xOffset,terrain.topY - yOffset);
            gfx_FillTriangle(terrain.topX - xOffset,terrain.topY -terrain.z - yOffset,
                            terrain.topX - xOffset,terrain.topY - yOffset,
                            terrain.x4 - xOffset,terrain.y4-terrain.z - yOffset);
        }   
        //front left
        gfx_SetColor(224);
        if (terrain.z < game.waterLevel) gfx_SetColor(16);
        
        //if next row is deeper than row, don't display front/left
        //last row should always show face
        if (terrain.z > nextRow.z || idx > 379) {    
            gfx_FillTriangle(
                terrain.x4 - xOffset, terrain.y4 - yOffset - nextRow.z, 
                terrain.x4 - xOffset, terrain.y4 - yOffset - terrain.z,
                terrain.x3 - xOffset, terrain.y3 - yOffset - nextRow.z
            );

            gfx_FillTriangle(
                terrain.x3 - xOffset, terrain.y3 - yOffset - nextRow.z,
                terrain.x3 - xOffset, terrain.y3 - yOffset - terrain.z,
                terrain.x4 - xOffset, terrain.y4 - yOffset - terrain.z
            );

            //draw full tile
            /*gfx_FillTriangle(
                terrain.x4 - xOffset, terrain.y4 - yOffset,
                terrain.x4 - xOffset, terrain.y4 - yOffset - (terrain.z),
                terrain.x3 - xOffset, terrain.y3 - yOffset
            );
            gfx_FillTriangle(
                terrain.x3 - xOffset, terrain.y3 - yOffset,
                terrain.x3 - xOffset, terrain.y3 - yOffset - (terrain.z),
                terrain.x4 - xOffset, terrain.y4 - yOffset - (terrain.z)
            );*/
        }

        //front right
        gfx_SetColor(32);
        if (terrain.z < game.waterLevel) gfx_SetColor(16);

        //full front right
        gfx_FillTriangle(
            terrain.x3 - xOffset,terrain.y3 - yOffset,
            terrain.x3 - xOffset,terrain.y3 - yOffset - terrain.z, 
            terrain.x2 - xOffset,terrain.y2 - yOffset - terrain.z
        );
        gfx_FillTriangle(
            terrain.x3 - xOffset,terrain.y3 - yOffset,
            terrain.x2 - xOffset,terrain.y2 - yOffset, 
            terrain.x2 - xOffset,terrain.y2 - yOffset - terrain.z
        );
        
        //attempt to optimize front right tile
        /*if (terrain.z > prev.z && (idx + 1) % 20 != 0) {
            gfx_FillTriangle(
                terrain.x3 - xOffset,terrain.y3 - yOffset - prev.z,
                terrain.x3 - xOffset,terrain.y3 - yOffset - terrain.z, 
                terrain.x2 - xOffset,terrain.y2 - yOffset - terrain.z
            );
            gfx_FillTriangle(
                terrain.x3 - xOffset,terrain.y3 - yOffset- prev.z,
                terrain.x2 - xOffset,terrain.y2 - yOffset- prev.z, 
                terrain.x2 - xOffset,terrain.y2 - yOffset - terrain.z
            );
        }*/
        
        
        //top
        gfx_SetColor(96);
        if (terrain.z < game.waterLevel) {
            /*gfx_SetColor(16);
            gfx_FillTriangle(terrain.x4 - xOffset, terrain.y4 - yOffset - terrain.z,
                        terrain.topX - xOffset, terrain.topY - yOffset - terrain.z,
                        terrain.x3 - xOffset, terrain.y3 - yOffset - terrain.z);

            gfx_FillTriangle(terrain.x3 - xOffset,terrain.y3 - terrain.z - yOffset,
                            terrain.topX - xOffset, terrain.topY - yOffset - terrain.z,
                            terrain.x2 - xOffset, terrain.y2 - yOffset - terrain.z);*/
            gfx_TransparentSprite(water_top,terrain.x4 - xOffset,terrain.topY - yOffset - terrain.z);
        } else if (terrain.z > 25 && game.waterLevel < 25) {
            gfx_SetColor(255);
            gfx_FillTriangle(terrain.x4 - xOffset, terrain.y4 - yOffset - terrain.z,
                        terrain.topX - xOffset, terrain.topY - yOffset - terrain.z,
                        terrain.x3 - xOffset, terrain.y3 - yOffset - terrain.z);

            gfx_FillTriangle(terrain.x3 - xOffset,terrain.y3 - terrain.z - yOffset,
                            terrain.topX - xOffset, terrain.topY - yOffset - terrain.z,
                            terrain.x2 - xOffset, terrain.y2 - yOffset - terrain.z);
            // gfx_TransparentSprite(snow_top,terrain.x4 - xOffset,terrain.topY - yOffset - terrain.z);
        } else {
            gfx_TransparentSprite(top,terrain.x4 - xOffset,terrain.topY - yOffset - terrain.z);
        }
        /*if (terrain.z < game.waterLevel) gfx_SetColor(16);
        //snow level at 25 should be modified by game
        if (terrain.z > 25 && game.waterLevel < 25) gfx_SetColor(255);
        gfx_FillTriangle(terrain.x4 - xOffset, terrain.y4 - yOffset - terrain.z,
                        terrain.topX - xOffset, terrain.topY - yOffset - terrain.z,
                        terrain.x3 - xOffset, terrain.y3 - yOffset - terrain.z);

        gfx_FillTriangle(terrain.x3 - xOffset,terrain.y3 - terrain.z - yOffset,
                        terrain.topX - xOffset, terrain.topY - yOffset - terrain.z,
                        terrain.x2 - xOffset, terrain.y2 - yOffset - terrain.z);*/
    }
}

void drawBuildings() {
    int idx;
    for (idx = 0; idx < 400; idx++) {
        terrain_t* terrain = &(terrainTiles[idx]);
        //if building is underwater, then delete building
        if (terrain->building == 1) {
            if (game.waterLevel >= terrain->z) {
                terrain->building = 0;

                //remove any multiplier
                if (terrain->buildingID == 2301) {
                    game.waterMultiplier--;
                } else if (terrain->buildingID == 1602) {
                    game.evapMultiplier--;
                } else if (terrain->buildingID == 801) {
                    game.population -= 10;
                } else if (terrain->buildingID == 1301) {
                    game.income--;
                }
            }
        }

        if (terrain->building == 1) {
            //set colors
            gfx_SetPalette(global_palette, sizeof_global_palette, 0);
            gfx_SetTransparentColor(0);
        
            if (terrain->buildingID == 1601) {
                gfx_TransparentSprite(solar_panel, terrain->topX - xOffset - 12, terrain->y3 - yOffset - 26 - terrain->z);
            } else if (terrain->buildingID == 1602) {
                gfx_TransparentSprite(power_plant, terrain->topX - xOffset - 12, terrain->y3 - yOffset - 26 - terrain->z);
            } else if (terrain->buildingID == 2301) {
                gfx_TransparentSprite(water, terrain->topX - xOffset - 12, terrain->y3 - yOffset - 26 - terrain->z);
            } else if (terrain->buildingID == 1301) {
                gfx_TransparentSprite(iron_mine, terrain->topX - xOffset - 12, terrain->y3 - yOffset - 26 - terrain->z);
            } else if (terrain->buildingID == 801) {
                gfx_TransparentSprite(house, terrain->topX - xOffset - 12, terrain->y3 - yOffset - 26 - terrain->z);
            }
        }
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
        gfx_PrintStringXY("YEAR    ",310 - gfx_GetStringWidth("YEAR 2020"),12);
        gfx_PrintInt(2020 + game.year,1);
        WhiText();
        gfx_PrintStringXY("BUILD",50 - gfx_GetStringWidth("BUILD") / 2,222);
        gfx_PrintStringXY("STATS",160 - gfx_GetStringWidth("STATS") / 2,222);
        YelText();
        gfx_PrintStringXY("MAP",270 - gfx_GetStringWidth("MAP") / 2,222);

    } else if (game.menuSelected == 1) {
        //gold 
        //population
        gfx_SetColor(229);
        gfx_FillCircle(11,14,8);
        YelText();
        gfx_PrintStringXY("",25,12);
        gfx_PrintInt(game.genCoin,1);
        WhiText();
        gfx_PrintStringXY("YEAR    ",310 - gfx_GetStringWidth("YEAR 2020"),12);
        gfx_PrintInt(2020 + game.year,1);

        //water
        //heat
        //population
    }
    if (game.menuSelected == 1) {
        YelText();
        gfx_PrintStringXY("BUILD",50 - gfx_GetStringWidth("BUILD") / 2,222);
        WhiText();
        gfx_PrintStringXY("STATS",160 - gfx_GetStringWidth("STATS") / 2,222);
        gfx_PrintStringXY("MAP",270 - gfx_GetStringWidth("MAP") / 2,222);
    } else if (game.menuSelected == 2) {
        WhiText();
        gfx_PrintStringXY("BUILD",50 - gfx_GetStringWidth("BUILD") / 2,222);
        YelText();
        gfx_PrintStringXY("STATS",160 - gfx_GetStringWidth("STATS") / 2,222);
        WhiText();
        gfx_PrintStringXY("MAP",270 - gfx_GetStringWidth("MAP") / 2,222);
    } else if (game.menuSelected == 3) {
        WhiText();
        gfx_PrintStringXY("BUILD",50 - gfx_GetStringWidth("BUILD") / 2,222);
        gfx_PrintStringXY("STATS",160 - gfx_GetStringWidth("STATS") / 2,222);
        YelText();
        gfx_PrintStringXY("MAP",270 - gfx_GetStringWidth("MAP") / 2,222);
    }
}

void doRedraw() {
    //act on the screen
    gfx_SetDrawScreen();
    //redraw buffered background
    gfx_Blit(1);
    drawCursor();
    //if (game.buildingSelected != 0) drawBuildingCursor();
}

void updateMap() {
    gfx_SetDraw(1);
    gfx_SetDrawBuffer();
    gfx_ZeroScreen();
    drawTerrain();
    //in update clim, find if building tiles changed, if so del building
    //updateClimate
    drawBuildings();
    drawTabs();
    gfx_SetDrawScreen();
    drawCursor();
}

void drawCursor() {
    terrain_t* terrain = &(terrainTiles[game.tileSelected]);
    gfx_SetColor(255);
    //change cursor to building selected
    //gfx_TransparentSprite(solar_panel, game.cursorx,game.cursory);
    if (game.buildingSelected == 0) {
        gfx_Circle(game.cursorx,game.cursory,5);
        gfx_SetColor(0);
        gfx_SetPixel(game.cursorx,game.cursory);
        gfx_SetPixel(game.cursorx - 1,game.cursory);
        gfx_SetPixel(game.cursorx + 1,game.cursory);
        gfx_SetPixel(game.cursorx,game.cursory - 1);
        gfx_SetPixel(game.cursorx,game.cursory + 1);
    } else {
        gfx_SetColor(255);

        //does not work well on water
        if (terrain->z > game.waterLevel) {
            gfx_Line(terrain->topX - xOffset,terrain->topY - yOffset - terrain->z,terrain->x2 - xOffset,terrain->y2 - yOffset - terrain->z);
            gfx_Line(terrain->x2 - xOffset,terrain->y2 - yOffset - terrain->z,terrain->x3 - xOffset,terrain->y3 - yOffset - terrain->z);
            gfx_Line(terrain->x3 - xOffset,terrain->y3 - yOffset - terrain->z,terrain->x4 - xOffset,terrain->y4 - yOffset - terrain->z);
            gfx_Line(terrain->x4 - xOffset,terrain->y4 - yOffset - terrain->z,terrain->topX - xOffset,terrain->topY - yOffset - terrain->z);
        
            if (game.buildingSelected == 1301) {
                gfx_TransparentSprite(iron_mine, terrain->topX - 12 - xOffset, terrain->y3 - 26 - yOffset - terrain->z);
            } else if (game.buildingSelected == 1601) {
                gfx_TransparentSprite(solar_panel, terrain->topX - 12 - xOffset, terrain->y3 - 26 - yOffset - terrain->z);
            } else if (game.buildingSelected == 1602) {
                gfx_TransparentSprite(power_plant, terrain->topX - 12 - xOffset, terrain->y3 - 26 - yOffset - terrain->z);
            } else if (game.buildingSelected == 2301) {
                gfx_TransparentSprite(water,terrain->topX - 12 - xOffset, terrain->y3 - 26 - yOffset - terrain->z);
            } else if (game.buildingSelected == 801) {
                gfx_TransparentSprite(house, terrain->topX - xOffset - 12, terrain->y3 - yOffset - 26 - terrain->z);
            }  
        } else {
            gfx_Line(terrain->topX - xOffset,terrain->topY - yOffset,terrain->x2 - xOffset,terrain->y2 - yOffset);
            gfx_Line(terrain->x2 - xOffset,terrain->y2 - yOffset,terrain->x3 - xOffset,terrain->y3 - yOffset);
            gfx_Line(terrain->x3 - xOffset,terrain->y3 - yOffset,terrain->x4 - xOffset,terrain->y4 - yOffset);
            gfx_Line(terrain->x4 - xOffset,terrain->y4 - yOffset,terrain->topX - xOffset,terrain->topY - yOffset);

            if (game.buildingSelected == 1301) {
                gfx_TransparentSprite(iron_mine, terrain->topX - 12 - xOffset, terrain->y3 - 26 - yOffset);
            } else if (game.buildingSelected == 1601) {
                gfx_TransparentSprite(solar_panel, terrain->topX - 12 - xOffset, terrain->y3 - 26 - yOffset);
            } else if (game.buildingSelected == 1602) {
                gfx_TransparentSprite(power_plant, terrain->topX - 12 - xOffset, terrain->y3 - 26 - yOffset);
            } else if (game.buildingSelected == 2301) {
                gfx_TransparentSprite(water,terrain->topX - 12 - xOffset, terrain->y3 - 26 - yOffset);
            } else if (game.buildingSelected == 801) {
                gfx_TransparentSprite(house, terrain->topX - xOffset - 12, terrain->y3 - yOffset - 26);
            }
        }

        if (terrain->z <= game.waterLevel) {
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
    }
}

//used for debugging
void dispStats() {
    terrain_t terrain = terrainTiles[game.tileSelected];
    WhiText();
    gfx_PrintStringXY("Index",10,50);
    gfx_PrintInt(game.tileSelected,1);
    gfx_PrintStringXY("Building",10,60);
    gfx_PrintInt(terrain.building,1);
}

void drawScreen() {
    bool canQuit;
    uint8_t buildingSelected = 0;
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(0);
    key = kb_Data[6];
    //gfx_SetDrawScreen();
    gfx_ZeroScreen();
    if (game.menuSelected == 0) {
        gfx_SetDrawScreen();
        gfx_ZeroScreen();
        //updateClimate();
        drawTerrain();
        //place inside of draw terrain function?
        drawBuildings();
        //don't draw tabs until building is placed
        if (game.buildingSelected == 0) {
            drawTabs();
        }
        
        gfx_Blit(0);
        gfx_SwapDraw();
        gfx_SetDrawScreen();
        drawCursor();
    } else if (game.menuSelected == 1) {
        //gfx_Begin();
        canQuit = false;
        gfx_SetPalette(global_palette, sizeof_global_palette, 0);
        gfx_SetTransparentColor(0);
        do {
            kb_Scan();
            gfx_SetDrawBuffer();
            gfx_ZeroScreen();
            drawTabs();
            if (kb_Data[7] & kb_Up && buildingSelected > 0) {
                buildingSelected--;
                delay(150);
            } else if (kb_Data[7] & kb_Down && buildingSelected < 4) {
                buildingSelected++;
                delay(150);
            } 
            
            gfx_TransparentSprite(solar_panel, 10, 40);
            gfx_TransparentSprite(power_plant, 10, 70);
            gfx_TransparentSprite(water, 10, 100);
            gfx_TransparentSprite(iron_mine, 10, 130);
            gfx_TransparentSprite(house,10,160);
            //change cursor to building
            WhiText();
            gfx_PrintStringXY("Solar Panel", 40, 50);
            gfx_PrintStringXY("Power Plant", 40, 80);
            gfx_PrintStringXY("Ice to Water Generator", 40, 110);
            gfx_PrintStringXY("Iron Mine", 40, 140);
            gfx_PrintStringXY("Habitat", 40, 170);
            if (buildingSelected == 0) {
                game.buildingSelected = 1601;
                YelText();
                gfx_PrintStringXY("Solar Panel", 40, 50);
                WhiText();
            } else if (buildingSelected == 1) {
                game.buildingSelected = 1602;
                YelText();
                gfx_PrintStringXY("Power Plant", 40, 80);
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
            } else if (buildingSelected == 4) {
                game.buildingSelected = 801;
                YelText();
                gfx_PrintStringXY("Habitat", 40, 170);
            }
            gfx_SwapDraw();
            //need canquit variable here, clear to go back to map
            //if (kb_Data[6] & kb_Clear) 
            if (kb_Data[1] & kb_Zoom) {
                game.menuSelected = 2;
                game.buildingSelected = 0;
                canQuit = true;
            } else if (kb_Data[1] & kb_Graph) {
                game.menuSelected = 0;
                game.buildingSelected = 0;
                canQuit = true;
            }
            if (kb_Data[6] & kb_Enter) {
                game.menuSelected = 0;
                canQuit = true;
                //check if building is selected to decide if to draw tabs
            } 
            /*else if (kb_Data[6] & kb_Clear) {
                //this closes problem, temporarily remove
                game.menuSelected = 0;
                game.buildingSelected = 0;
                canQuit = true;
            }*/
        } while(canQuit == false);
        drawScreen();
    } else if (game.menuSelected == 2) {
        //statistics
        //water
        //population
        canQuit = false;
        //gfx_Begin();
        do {
            kb_Scan();
            gfx_SetDrawBuffer();
            gfx_ZeroScreen();
            drawTabs();

            //handle keys here
            if (kb_Data[6] & kb_Add) {
                game.waterLevel++;
                delay(200);
            } else if (kb_Data[6] & kb_Sub) {
                game.waterLevel--;
                delay(200);
            }
            
            gfx_SetTextScale(2,2);
            YelText();
            gfx_PrintStringXY("WORLD STATS",10,50);
            WhiText();
            gfx_SetTextScale(1,1);
            gfx_PrintStringXY("Gen Coins/hr: ",10,75);
            gfx_PrintInt(game.income,1);
            gfx_PrintStringXY("Population: ",10,90);
            gfx_PrintInt(game.population,1);
            gfx_PrintStringXY("Heat Multiplier: ",10,105);
            gfx_PrintInt(game.evapMultiplier,1);
            gfx_PrintStringXY("Water Multiplier: ",10,120);
            gfx_PrintInt(game.waterMultiplier,1);
            gfx_SetTextScale(2,2);
            YelText();
            gfx_PrintStringXY("EDIT TERRAIN",10,140);
            WhiText();
            gfx_SetTextScale(1,1);
            gfx_PrintStringXY("Water Level: ",10,165);
            gfx_PrintInt(game.waterLevel,1);
            YelText();
            gfx_PrintStringXY("+/-",140,165);
            //snow level
            //terrain settings
            gfx_SwapDraw();
            if (kb_Data[1] & kb_Graph) {
                game.menuSelected = 0;
                canQuit = true;
            } else if (kb_Data[1] & kb_Yequ) {
                game.menuSelected = 1;
                canQuit = true;
            }
        } while (canQuit == false);
        drawScreen();
    } else if (game.menuSelected == 3) {
        //exit
    }
}

//manage all cursor functions
void doCursor() {
    key = kb_Data[7];
    //erase cursor
    //make sure building isn't selected
    if (game.buildingSelected == 0) {
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
            updateMap();
            game.cursory = 50;
            doRedraw();
        } else if (game.cursory + 5 > 201) {
            yOffset = yOffset + 20;
            updateMap();
            game.cursory = 190;
            doRedraw();
        } else if (game.cursorx - 5 < -5) {
            xOffset = xOffset - 20;
            updateMap();
            game.cursorx = 20;
            doRedraw();
        } else if (game.cursorx > 315) {
            xOffset = xOffset + 20;
            updateMap();
            game.cursorx = 300;
            doRedraw();
        }
    } else {
        if (key == kb_Up && game.tileSelected > 19) {
            delay(150);
            game.tileSelected -= 20;
            doRedraw();
        } else if (key == kb_Down && game.tileSelected < 379) {
            delay(150);
            game.tileSelected += 20;
            doRedraw();
        } else if (key == kb_Left && game.tileSelected != 0) {
            delay(150);
            game.tileSelected--;
            doRedraw();
        } else if (key == kb_Right && game.tileSelected != 399) {
            delay(150);
            game.tileSelected++;
            doRedraw();
        }
    }
}

void handleKeys() {
    key = kb_Data[1];

    if (key & kb_Yequ) {
        //build screen
        game.menuSelected = 1;
        drawScreen();
        
    } else if (key & kb_Zoom) {
        //stats- probably change to zoom in, have stats be third tab
        game.menuSelected = 2;
        drawScreen();
    } else if (key & kb_Graph) {
        game.menuSelected = 0;
        //game.tileSelected++;
    }

    /*if (kb_Data[6] & kb_Clear) {
        //save game? 
    }*/

    if ((kb_Data[1] & kb_Mode) && game.dispStats == false) {
        game.dispStats = true;
    } else {
        game.dispStats = false;
    }

    if (kb_Data[6] & kb_Add && game.buildingSelected == 0) {
        game.menuSelected = 1;
        drawScreen();
    }
    if (kb_Data[6] & kb_Add && game.buildingSelected != 0) {
        terrain_t* terrain = &(terrainTiles[game.tileSelected]);
        if (terrain->z > game.waterLevel && terrain->building == false) {
            terrain->building = true;
            terrain->buildingID = game.buildingSelected;
            if (game.buildingSelected == 2301) {
                //ice to water
                game.waterMultiplier++;
            } else if (game.buildingSelected == 801) {
                game.population += 10;
            } else if (game.buildingSelected == 1602) {
                game.evapMultiplier++;
            } else if (game.buildingSelected == 1301) {
                game.income++;
            }
            game.buildingSelected = 0;
            game.menuSelected = 0;
            //update multipliers, population, gold
            //update map
            /*//delay so doesn't trigger menu again
            delay(150);
            drawTabs();*/

            // add rising edge detector instead of delaying
            updateMap();
            doRedraw();
        }
    }
}

void runGame() {
    gfx_Begin();

    if (game.seed == 0) {
        srand(rtc_Time());
        game.seed = randInt(-10000,10000);
        
        //reset all parameters - no cheaters
        game.waterLevel = 0;
        game.population = 0;
    }
    //game.seed = 25;
    createTerrain();
    //free();
    //may cause issue with placing buildings
    game.cursorx = 160;
    game.cursory = 120;
    game.buildingSelected = 0;
    game.canPress = true;
    game.menuSelected = 0;
    gfx_SetDrawScreen();
    gfx_ZeroScreen();
    drawTerrain();
    drawBuildings();
    
    drawTabs();
    gfx_Blit(0);
    gfx_SwapDraw();
    gfx_SetDrawScreen();
    drawCursor();
}


void updateStats() {
    //maybe force ice to water gen to build on ice
    //total climate variable that sums the two forces

    //maybe have natural environment factors
    //since it's Mars, watermultiplier starts in the negatives?
    
    game.numLoops++;
    //income
    game.genCoin += game.income / 10;
    game.climate = game.waterMultiplier - game.evapMultiplier;
    //hours in mars year - 16488
    if (game.numLoops >= 6000) {
        game.year++;
        //3500 loops is 20 seconds
        if (game.climate != 0) {
            if (game.climate > 0) {
            game.waterLevel++;
            } else if (game.climate < 0 && game.waterLevel > 0) {
                //game.variability - change in height between tiles
                //waterlevel is uint8, waterlevel would go to 255
                game.waterLevel--;
            }
            updateMap();
            doRedraw();
        }
        
        game.numLoops = 0;
    }
}

void main() {
    bool canQuit = 0;
    LoadData();
    //set tile selected to center for building placement
    game.tileSelected = 200;
    game.doCursor = true;
    runGame();
    do {
        kb_Scan();
        //buffer, map, player, swap
        if (game.doCursor == true) doCursor();
        drawCursor();
        if (game.menuSelected == 0 && game.buildingSelected == 0) updateStats();
        if (game.dispStats == true) dispStats();
        handleKeys();
        if (kb_Data[6] & kb_Clear) {
            if (game.menuSelected != 1) {
                canQuit = true;
            }
        }
    } while (!(canQuit));
    game.save = true;
    //CreateSave();
    SaveData();
    gfx_End();
}