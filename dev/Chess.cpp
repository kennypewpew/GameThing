#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Window.h"
#include "GlLayer.h"
#include "Types.h"
#include "Utility.h"
#include "Visuals.h"

// Chess for practice

const unsigned int DISP_WIDTH = 720;
const unsigned int DISP_HEIGHT = 720;

enum PieceType {
    PIECE_INVALID
  , PIECE_PAWN
  , PIECE_ROOK
  , PIECE_KNIGHT
  , PIECE_BISHOP
  , PIECE_QUEEN
  , PIECE_KING
  , PIECE_TOTAL
};

enum PlayerColor {
    PLAYER_INVALID
  , PLAYER_WHITE
  , PLAYER_BLACK
  , PLAYER_TOTAL
};

glm::vec3 PlayerColorVals[] = {
    glm::vec3(1.,0.,0.)
  , glm::vec3(0.9,0.9,0.9)
  , glm::vec3(0.5,0.20,0.1)
  , glm::vec3(1.,0.,0.)
};

class ChessPiece {
 public:
  int xPos;
  PieceType type;
  PlayerColor color;

  ChessPiece() : xPos(-1), type(PIECE_INVALID), color(PLAYER_INVALID) {}
  ChessPiece(int x, PieceType p, PlayerColor c) : xPos(x), type(p), color(c) {}
};

Pos2D PixelsToPosition(int xPos,int yPos) {
  int size = 8;
  int xBlock = DISP_WIDTH / size;
  int yBlock = DISP_HEIGHT / size;
  Pos2D res;
  res.x = xPos / xBlock;
  res.y = yPos / yBlock;
  res.y = size - res.y - 1;

  return res;
}

class ChessMap;
bool QuitMap(const SDL_Event &e , ChessMap* mp);

class ChessMap {
 public:
  bool quit = false;

  bool selected = false;
  Pos2D selection;
  std::vector<Pos2D> allowedMoves;

  glm::mat4 view;
  glm::mat4 zoom;
  std::vector<glm::vec3> v_grid, v_tiles, v_pieces, v_hl, v_active; // Vertices
  std::vector<GLuint> i_grid, i_tiles, i_pieces, i_hl, i_active; // Incides
  std::vector<glm::vec2> t_pieces; // Textures
  std::vector<glm::vec3> c_tiles, c_pieces; // Colors

  std::vector<std::vector<ChessPiece>> pieceLocations;
  std::vector<ChessPiece> captured;

  AtlasInfo atlasWhite;
  bool whiteTurn = true;

  ChessMap() {}

  void HandleInputEvent( const SDL_Event &e ) {
      //if ( handlers.count(e.type) ) (handlers[e.type])(e,this);
      if ( e.type == SDL_KEYDOWN ) {
        switch (e.key.keysym.sym)
        {
          case SDLK_q:
            this->quit = true;
            break;
          default:
            break;
        }
      }
      else if ( uint8_t(e.type) == uint8_t(SDL_MOUSEBUTTONDOWN) ) {
        switch(e.button.button)
        {
          case SDL_BUTTON_LEFT:
            int xPos, yPos;
            SDL_GetMouseState(&xPos,&yPos);
            Pos2D p = PixelsToPosition(xPos,yPos);
            if ( selected ) {
              if ( MovePiece(selection.x,selection.y,p.x,p.y) ) ChangeTurn();
              Deselect();
            }
            else {
              SelectPiece(p.x,p.y);
            }
            break;
        }
      }
      else if ( e.type == SDL_MOUSEWHEEL ) {
        //if (e.wheel.y > 0 || e.mgesture.dDist > 0 ) {
        //  Zoom(+0.05f);
        //}
        //else if (e.wheel.y < 0 ) {
        //  Zoom(-0.05f);
        //}
      }
      //else if ( e.type == SDL_MULTIGESTURE ) {
      //  if ( e.mgesture.dDist > 0 ) {
      //    Zoom( e.mgesture.dDist/50. );
      //  }
      //  else if ( e.mgesture.dDist < 0 ) {
      //    Zoom( e.mgesture.dDist/50. );
      //  }
      //}
    }

  void InitializeActiveLayer() {
    float x1 = -1;
    float x2 =  1;
    float y1 = -1;
    float y2 = -0.97;
    AddSquare2D( x1, y1, x2, y2, v_active, i_active );
  }

  void CreateBoard() {
    glm::vec3 black(0.1,0.1,0.1);
    glm::vec3 white(1.,1.,1.);
    int size = 8;
    for ( int i = 0 ; i < size ; ++i ) {
      float x1 = float(  i  ) / float(size); x1 = x1 * 2. - 1.;
      float x2 = float((i+1)) / float(size); x2 = x2 * 2. - 1.;
      for ( int j = 0 ; j < size ; ++j ) {
        float y1 = float(  j  ) / float(size); y1 = y1 * 2. - 1.;
        float y2 = float((j+1)) / float(size); y2 = y2 * 2. - 1.;
        glm::vec3 &color = ((i+j) % 2) ? black : white;
        AddSquare2D( x1, y1, x2, y2, color, v_tiles, i_tiles, c_tiles );
      }
    }
    pieceLocations = std::vector<std::vector<ChessPiece>>(size,std::vector<ChessPiece>(size));
  }

  std::string StringifyPieceType(PieceType t) {
    if      ( t == PIECE_PAWN ) return "pawn";
    else if ( t == PIECE_ROOK ) return "rook";
    else if ( t == PIECE_KNIGHT ) return "knight";
    else if ( t == PIECE_BISHOP ) return "bishop";
    else if ( t == PIECE_QUEEN ) return "queen";
    else if ( t == PIECE_KING ) return "king";
    else return "error";
  }

  void HighlightSquare(int x, int y) {
    int size = 8;
    float x1 = float(  x  ) / float(size); x1 = x1 * 2. - 1.;
    float x2 = float((x+1)) / float(size); x2 = x2 * 2. - 1.;
    float y1 = float(  y  ) / float(size); y1 = y1 * 2. - 1.;
    float y2 = float((y+1)) / float(size); y2 = y2 * 2. - 1.;

    v_hl.push_back(glm::vec3(x1,y1,0));
    v_hl.push_back(glm::vec3(x1,y2,0));
    v_hl.push_back(glm::vec3(x2,y2,0));
    v_hl.push_back(glm::vec3(x2,y1,0));
    i_hl.push_back(0);
    i_hl.push_back(1);
    i_hl.push_back(2);
    i_hl.push_back(0);
    i_hl.push_back(2);
    i_hl.push_back(3);
  }

  bool CanEnter(int x, int y) {
    int size = 8;

    if (  x >= size || y >= size
       || x < 0 || y < 0 ) return false;

    if ( PIECE_INVALID != pieceLocations[x][y].type ) return false;

    return true;
  }

  bool CanCapture(int x, int y, PlayerColor c) {
    int size = 8;

    if (  x >= size || y >= size
       || x < 0 || y < 0 ) return false;

    if ( PIECE_INVALID != pieceLocations[x][y].type && c != pieceLocations[x][y].color ) return true;

    return false;
  }

  void FillMovesPawn(ChessPiece p, int x, int y) {
    if ( PIECE_INVALID == p.type ) return;

    int size = 8;
    //int xDir = 1;
    int yDir = 1;
    int yHome = 1;
    if ( PLAYER_WHITE == p.color ) {
      yDir =  1;
      yHome = 1;
    }
    if ( PLAYER_BLACK == p.color ) {
      yDir = -1;
      yHome = size - 2;
    }
    if ( yHome == y && CanEnter(x,y+2*yDir) ) {
      allowedMoves.push_back(Pos2D( x , y+2*yDir ));
    }
    if ( CanEnter(x,y+1*yDir) ) {
      allowedMoves.push_back(Pos2D( x , y+1*yDir ) );
    }
    if ( CanCapture(x+1,y+1*yDir,p.color) ) {
      allowedMoves.push_back(Pos2D( x+1 , y+1*yDir ) );
    }
    if ( CanCapture(x-1,y+1*yDir,p.color) ) {
      allowedMoves.push_back(Pos2D( x-1 , y+1*yDir ) );
    }
  } 

  void FillMovesDirectional(int x, int y, Pos2D *dirs, int nDirs, PlayerColor c, int max = 1000) {
    for ( int i = 0 ; i < nDirs ; ++i ) {
      bool blocked = false;
      int step = 1;
      while ( !blocked && step <= max ) {
        int xPos = x + step*dirs[i].x;
        int yPos = y + step*dirs[i].y;
        if ( CanEnter(xPos,yPos) ) {
          allowedMoves.push_back(Pos2D(xPos,yPos));
        }
        else {
          blocked = true;
          if ( CanCapture(xPos,yPos,c) ) allowedMoves.push_back(Pos2D(xPos,yPos));
        }
        ++step;
      }
    }
  }

  void FillMovesRook(ChessPiece p, int x, int y) {
    Pos2D directions[4] = { Pos2D(1,0) , Pos2D(-1,0) , Pos2D(0,1) , Pos2D(0,-1) };
    FillMovesDirectional(x,y,directions,4,p.color);
  } 

  void FillMovesKnight(ChessPiece p, int x, int y) {
    Pos2D directions[8] = { Pos2D( 2, 1) , Pos2D( 2,-1)
                          , Pos2D(-2, 1) , Pos2D(-2,-1)
                          , Pos2D(-1, 2) , Pos2D( 1, 2)
                          , Pos2D(-1,-2) , Pos2D( 1,-2)
    };
    FillMovesDirectional(x,y,directions,8,p.color,1);
  }

  void FillMovesBishop(ChessPiece p, int x, int y) {
    Pos2D directions[4] = { Pos2D(1,1) , Pos2D(-1,1) , Pos2D(-1,-1) , Pos2D(1,-1) };
    FillMovesDirectional(x,y,directions,4,p.color);
  } 

  void FillMovesQueen(ChessPiece p, int x, int y) {
    Pos2D directions[8] = { Pos2D(1,1) , Pos2D(-1,1) , Pos2D(-1,-1) , Pos2D(1,-1)
                          , Pos2D(1,0) , Pos2D(-1,0) , Pos2D(0,1) , Pos2D(0,-1) };
    FillMovesDirectional(x,y,directions,8,p.color);
  } 

  void FillMovesKing(ChessPiece p, int x, int y) {
    Pos2D directions[8] = { Pos2D(1,1) , Pos2D(-1,1) , Pos2D(-1,-1) , Pos2D(1,-1)
                          , Pos2D(1,0) , Pos2D(-1,0) , Pos2D(0,1) , Pos2D(0,-1) };
    FillMovesDirectional(x,y,directions,8,p.color,1);
  } 

  void GetAllowedMoves(int x, int y) {
    allowedMoves.clear();
    ChessPiece p = pieceLocations[x][y];

    if ( PIECE_INVALID == p.type ) return;
    else if ( PIECE_PAWN   == p.type ) FillMovesPawn  (p,x,y);
    else if ( PIECE_ROOK   == p.type ) FillMovesRook  (p,x,y);
    else if ( PIECE_KNIGHT == p.type ) FillMovesKnight(p,x,y);
    else if ( PIECE_BISHOP == p.type ) FillMovesBishop(p,x,y);
    else if ( PIECE_QUEEN  == p.type ) FillMovesQueen (p,x,y);
    else if ( PIECE_KING   == p.type ) FillMovesKing  (p,x,y);
  }

  void HighlightMoves(int i, int j) {
    GetAllowedMoves(i,j);

    for ( auto p : allowedMoves ) {
      int size = 8;
      int x = p.x;
      int y = p.y;

      float x1 = float(  x  ) / float(size); x1 = x1 * 2. - 1.;
      float x2 = float((x+1)) / float(size); x2 = x2 * 2. - 1.;
      float y1 = float(  y  ) / float(size); y1 = y1 * 2. - 1.;
      float y2 = float((y+1)) / float(size); y2 = y2 * 2. - 1.;

      size_t offset = v_hl.size();
      v_hl.push_back(glm::vec3(x1,y1,0));
      v_hl.push_back(glm::vec3(x1,y2,0));
      v_hl.push_back(glm::vec3(x2,y2,0));
      v_hl.push_back(glm::vec3(x2,y1,0));
      i_hl.push_back(offset + 0);
      i_hl.push_back(offset + 1);
      i_hl.push_back(offset + 2);
      i_hl.push_back(offset + 0);
      i_hl.push_back(offset + 2);
      i_hl.push_back(offset + 3);
    }
  }

  void SelectPiece(int x, int y) {
    if ( PIECE_INVALID == pieceLocations[x][y].type ) return;

    if ( whiteTurn && pieceLocations[x][y].color != PLAYER_WHITE ) return;
    if ( !whiteTurn && pieceLocations[x][y].color == PLAYER_WHITE ) return;

    selected = true;
    selection.x = x;
    selection.y = y;

    HighlightSquare(x,y);
    HighlightMoves(x,y);
  }

  void Deselect() {
    selected = false;
    v_hl.clear();
    i_hl.clear();
  }

  void AddPiece(int i, int j, PieceType t, PlayerColor c) {
    int size = 8;
    float x1 = float(  i  ) / float(size); x1 = x1 * 2. - 1.;
    float x2 = float((i+1)) / float(size); x2 = x2 * 2. - 1.;
    float y1 = float(  j  ) / float(size); y1 = y1 * 2. - 1.;
    float y2 = float((j+1)) / float(size); y2 = y2 * 2. - 1.;
    int initOffset = v_pieces.size();

    AddSquare2D( x1, y1, x2, y2, v_pieces, i_pieces);
    Box2Df coords = atlasWhite[StringifyPieceType(t)];
    AddTextureCoords( t_pieces, coords );
    for ( int i = 0 ; i < 4 ; ++i ) c_pieces.push_back(PlayerColorVals[c]);

    pieceLocations[i][j] = ChessPiece(initOffset,t,c);
  }

  void CapturePiece(ChessPiece p) {
    captured.push_back(p);

    int size = 8;
    int i = -2;
    int j = -2;
    float x1 = float(  i  ) / float(size); x1 = x1 * 2. - 1.;
    float x2 = float((i+1)) / float(size); x2 = x2 * 2. - 1.;
    float y1 = float(  j  ) / float(size); y1 = y1 * 2. - 1.;
    float y2 = float((j+1)) / float(size); y2 = y2 * 2. - 1.;
    SetSquare2DPos( x1 , y1 , x2 , y2 , v_pieces , p.xPos );
  }

  bool MoveIsAllowed(int x, int y) {
    for ( size_t i = 0 ; i < allowedMoves.size() ; ++i ) {
      if ( x == allowedMoves[i].x && y == allowedMoves[i].y ) return true;
    }
    return false;
  }

  bool MovePiece(int xi, int yi, int xf, int yf) {
    ChessPiece p = pieceLocations[xi][yi];
    ChessPiece pCap = pieceLocations[xf][yf];

    if ( p.type == PIECE_INVALID ) return false;
    if ( xi == xf && yi == yf ) return false;

    if ( !MoveIsAllowed(xf,yf) ) return false;

    if ( PIECE_INVALID != pCap.type ) {
      CapturePiece(pCap);
    }

    // TODO: pawn promotion
    // TODO: castling

    pieceLocations[xf][yf] = p;
    pieceLocations[xi][yi] = ChessPiece();

    int size = 8;
    int i = xf;
    int j = yf;
    float x1 = float(  i  ) / float(size); x1 = x1 * 2. - 1.;
    float x2 = float((i+1)) / float(size); x2 = x2 * 2. - 1.;
    float y1 = float(  j  ) / float(size); y1 = y1 * 2. - 1.;
    float y2 = float((j+1)) / float(size); y2 = y2 * 2. - 1.;
    SetSquare2DPos( x1 , y1 , x2 , y2 , v_pieces , p.xPos );

    return true;
  }

  void InitializePieces() {
    for ( int i = 0 ; i < 8 ; ++i ) {
      AddPiece(i,1,PIECE_PAWN,PLAYER_WHITE);
      AddPiece(i,6,PIECE_PAWN,PLAYER_BLACK);
    }
    AddPiece(0,0,PIECE_ROOK,PLAYER_WHITE);
    AddPiece(7,0,PIECE_ROOK,PLAYER_WHITE);
    AddPiece(0,7,PIECE_ROOK,PLAYER_BLACK);
    AddPiece(7,7,PIECE_ROOK,PLAYER_BLACK);

    AddPiece(1,0,PIECE_KNIGHT,PLAYER_WHITE);
    AddPiece(6,0,PIECE_KNIGHT,PLAYER_WHITE);
    AddPiece(1,7,PIECE_KNIGHT,PLAYER_BLACK);
    AddPiece(6,7,PIECE_KNIGHT,PLAYER_BLACK);

    AddPiece(2,0,PIECE_BISHOP,PLAYER_WHITE);
    AddPiece(5,0,PIECE_BISHOP,PLAYER_WHITE);
    AddPiece(2,7,PIECE_BISHOP,PLAYER_BLACK);
    AddPiece(5,7,PIECE_BISHOP,PLAYER_BLACK);

    AddPiece(3,0,PIECE_QUEEN,PLAYER_WHITE);
    AddPiece(3,7,PIECE_QUEEN,PLAYER_BLACK);

    AddPiece(4,0,PIECE_KING,PLAYER_WHITE);
    AddPiece(4,7,PIECE_KING,PLAYER_BLACK);
  }

  void InitializeAtlas() {
    atlasWhite = AtlasInfo(120,30,15,15);
    atlasWhite["pawn"] = atlasWhite(0,0);
    atlasWhite["rook"] = atlasWhite(0,1);
    atlasWhite["knight"] = atlasWhite(1,1);
    atlasWhite["bishop"] = atlasWhite(2,1);
    atlasWhite["queen"] = atlasWhite(3,1);
    atlasWhite["king"] = atlasWhite(4,1);
  }

  void ChangeTurn() {
    whiteTurn = !whiteTurn;

    int dir = whiteTurn ? -1 : 1;

    float x1 = -1;
    float x2 =  1;
    float y1 = dir * 1;
    float y2 = dir * 0.97;
    SetSquare2DPos( x1, y1, x2, y2, v_active, 0 );
  }

  int MainLoop(int argc, char **argv) {
    Shader shaderInputColor( "shader/inputColor.vs" , "shader/inputColor.fs" );
    Shader shaderSolidColor( "shader/solidColor.vs" , "shader/solidColor.fs" );
    Shader shaderTexture( "shader/inputTexture.vs" , "shader/inputTexture.fs" );
    Shader shaderColoredTexture( "shader/varInputTexture.vs" , "shader/varColoredTexture.fs" );

    InitializeAtlas();
    CreateBoard();
    InitializePieces();
    InitializeActiveLayer();

    // Colored tile IDs for clicking on tiles
    GlLayer boardLayer( shaderInputColor );
    boardLayer.AddInput( "position" , 3 );
    boardLayer.AddInput( "incolor" , 3 );
    boardLayer.SetUniform( "zoom" , glm::mat4(1.) );
    boardLayer.SetUniform( "shift" , glm::mat4(1.) );
    boardLayer.SetUniform( "view" , glm::mat4(1.) );
    boardLayer.BindCopyVB(this->v_tiles,3,0);
    boardLayer.BindCopyVB(this->c_tiles,3,1);
    boardLayer.BindCopyIB(this->i_tiles);

    GlLayer pieceLayer( shaderColoredTexture );
    pieceLayer.AddInput( "position" , 3 );
    pieceLayer.AddInput( "texColor" , 3 );
    pieceLayer.AddInput( "texCoord" , 2 );
    pieceLayer.SetUniform( "zoom" , glm::mat4(1.) );
    pieceLayer.SetUniform( "shift" , glm::mat4(1.) );
    pieceLayer.SetUniform( "view" , glm::mat4(1.) );
    pieceLayer.BindCopyVB(this->v_pieces,3);
    pieceLayer.BindCopyVB(this->c_pieces,3,1);
    pieceLayer.BindCopyVB(this->t_pieces,2,2);
    pieceLayer.BindCopyIB(this->i_pieces);
    pieceLayer.SetTexture("applyTexture","texCoord","assets/chess/white.png",GL_RGBA);

    GlLayer hlLayer( shaderSolidColor );
    hlLayer.AddInput( "position" , 3 );
    hlLayer.SetUniform( "incolor" , glm::vec4(0.,1.,0.,0.5) );
    hlLayer.SetUniform( "zoom" , glm::mat4(1.) );
    hlLayer.SetUniform( "shift" , glm::mat4(1.) );
    hlLayer.SetUniform( "view" , glm::mat4(1.) );
    hlLayer.BindCopyVB(this->v_hl,3,0);
    hlLayer.BindCopyIB(this->i_hl);

    GlLayer showActiveLayer( shaderSolidColor );
    showActiveLayer.AddInput( "position" , 3 );
    showActiveLayer.SetUniform( "incolor" , glm::vec4(1.,0.,0.,0.5) );
    showActiveLayer.SetUniform( "zoom" , glm::mat4(1.) );
    showActiveLayer.SetUniform( "shift" , glm::mat4(1.) );
    showActiveLayer.SetUniform( "view" , glm::mat4(1.) );
    showActiveLayer.BindCopyVB(this->v_active,3,0);
    showActiveLayer.BindCopyIB(this->i_active);

    while (!this->quit) {
      SDL_Event event;
      while (SDL_PollEvent(&event) > 0) {
        HandleInputEvent( event );
      }

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      boardLayer.UseThisLayer();
      glDrawElements(GL_TRIANGLES,this->i_tiles.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      pieceLayer.UseThisLayer();
      pieceLayer.BindCopyVB(this->v_pieces,3);
      glDrawElements(GL_TRIANGLES,this->i_pieces.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      hlLayer.UseThisLayer();
      hlLayer.SetUniform( "incolor" , glm::vec4(0.,1.,0.,0.5) );
      hlLayer.BindCopyVB(this->v_hl,3,0);
      hlLayer.BindCopyIB(this->i_hl);
      glDrawElements(GL_TRIANGLES,this->i_hl.size()*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      showActiveLayer.UseThisLayer();
      showActiveLayer.SetUniform( "incolor" , glm::vec4(1.,0.,0.,0.5) );
      showActiveLayer.BindCopyVB(this->v_active,3,0);
      showActiveLayer.BindCopyIB(this->i_active);
      glDrawElements(GL_TRIANGLES,(this->i_active.size())*sizeof(glm::vec2),GL_UNSIGNED_INT,NULL);

      SwapWindows();
    }

    return 0;
  }
};

int main(int argc, char **argv) {
  InitializeSDL("Chess", DISP_WIDTH, DISP_HEIGHT);

  ChessMap chess;
  return chess.MainLoop(argc, argv);
};
