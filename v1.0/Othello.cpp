// Othello.cpp : Defines the entry point for the application.
// and the main window controls.

#include "stdafx.h"
#include "Common.h"
#include "Othello.h"
#include "LoadSave.h"
#include "Bit.h"
#include "Moves.h"
#include "Comp.h"
#include "resource.h"

// Global Variables:
TCHAR szTitle[] = "Col's Othello";				// The title bar text
TCHAR szWindowClass[] = "Col's Othello";  // The (other) title bar text
int iHistoryPos = 0, iHistoryMax = 0;
int History[64];
BITBOARD Undo[64];
bool bComputer[2] = {false, true}, bShowPossible = true, bCalculating = false, bGameOver = false, bFlicker = false;
float BorderX = 0.04f, BorderY = 0.04f, TopBorder = 0.2f, flicker = PIF*8.0f;
int WindowWidth, WindowHeight, g_iBase, MaxDepth = 6, SearchTime = 10;
BITBOARD PossibleMoves = EMPTY;
HWND hWndMain;
HINSTANCE g_hInstance;
HMENU g_hMenu;
HDC BoardDC;
BoardType Board;
GLYPHMETRICSFLOAT Graphics_gmf[256];   // Storage For Information About Our Font
GLUquadricObj *Graphics_quadObj;

// External definitions
extern BITBOARD Mask[64];
extern int g_BestMove, g_ThinkingMove, g_Depth;
extern bool bAbortFlag;



// WinMain function is the application entry point
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow) {
  MSG msg;

  // Set up a global instance handle
  g_hInstance = hInstance;

  // Initialise the boards
  Randomise();
  SetupPrecalculatedData();
  ResetBoard(&Board);
  PossibleMoves = GenerateMoves(&Board);

  // Setup the main screen
  SetupMainWindow();

  // Set the window focus on the main window
  SetFocus(hWndMain);

  // Disable the undo/redo menu items
  EnableMenuItem(g_hMenu,IDM_EDIT_UNDO,MF_GRAYED);
  EnableMenuItem(g_hMenu,IDM_EDIT_REDO,MF_GRAYED);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
//    if(!IsDialogMessage(msg.hwnd, &msg)) {
 	  	TranslateMessage(&msg);
    	DispatchMessage(&msg);
//		}
    if(msg.message == WM_QUIT) {
      DestroyWindow( hWndMain );
      return (int)msg.wParam;
    }
  }

  return (int)msg.wParam;
}

#define HEARTBEAT_TIMER (1)

// Main window messaging function
INT_PTR CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  int wmId,wmEvent,nhx,nhy;
  static int mx,my,omx,omy,hx,hy;
  static bool bRunning = false, bSkipTimer = false;
  float xstep, ystep;
  HDC hDC;
  PAINTSTRUCT ps;
  static HGLRC hRC;

  switch (message) {
    case WM_INITDIALOG:
      return TRUE;
    case WM_CREATE:
      // Get a Device Context ... 
      BoardDC = GetDC(hWnd);
      hRC = Graphics_SetupWindow(hWnd,BoardDC);
      // Set up the viewport
      SetupViewport(hWnd);
      // Set a housekeeping timer
      SetTimer(hWnd,HEARTBEAT_TIMER,100,NULL);
      // Menu options
      CheckMenuItem(GetMenu(hWnd),IDM_OPTIONS_POSSIBLE,MF_CHECKED);
      CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_6,MF_CHECKED);
      CheckMenuItem(GetMenu(hWnd),IDM_TIME_10S,MF_CHECKED);
      CheckMenuItem(GetMenu(hWnd),IDM_OPTIONS_COMPUTERWHITE,MF_CHECKED);
      CheckMenuItem(GetMenu(hWnd),IDM_OPTIONS_FORCE,MF_GRAYED);
      break;
    case WM_MOUSEMOVE:  // Mouse has moved
      omx = mx;
      omy = my;
      mx  = LOWORD(lParam); 
      my  = HIWORD(lParam);
      xstep = (float)WindowWidth * (2.0f - (BorderX*2.0f)) / 16.0f;
      ystep = (float)WindowHeight * (2.0f - (BorderY*2.0f + TopBorder)) / 16.0f;
      nhx = (int)(((float)mx - (float)WindowWidth*BorderX/2.0f) / xstep);
      nhy = (int)(((float)my - (float)WindowHeight*(BorderY + TopBorder)/2.0f) / ystep);
      if (GetKeyState('A') < 0) {
        mx = mx;
      }
      if (nhx != hx || nhy != hy) {
        hx = nhx; hy = nhy;
        DisplayBoard(&Board,hx,hy);       
      }
      break;
    case WM_LBUTTONUP:  // L-mouse click.  Check for new box selection
      // Deal with a click (usually an attempted move)
      if (bGameOver) break;
      MouseClick(&Board,hx,hy);
      PossibleMoves = GenerateMoves(&Board);
      DisplayBoard(&Board,hx,hy);
      break;
    case WM_SIZE:
      SetupViewport(hWnd);
      DisplayBoard(&Board,hx,hy);
      break;
    case WM_TIMER:
      if (bGameOver || bSkipTimer) break;
      bSkipTimer = true;
      // If there aren't any moves then skip this turn. Check for game over.
      if (PossibleMoves == EMPTY) {
        if (iHistoryPos > 0 && History[iHistoryPos-1] == -1) {
          GameOver();
        }
        else {
          if (!bComputer[(Board.side+1)/2]) MessageBox(hWndMain,"No moves available. Press 'OK' to pass","No moves",MB_OK);
          Board.side = -Board.side;
          Undo[iHistoryPos] = EMPTY;
          History[iHistoryPos++] = -1;
          iHistoryMax = iHistoryPos;
          PossibleMoves = GenerateMoves(&Board);
        }
      }
      // Is it the computer's turn to move?
      if (bComputer[(Board.side+1)/2] && !bCalculating) {
        // Start the thread to calculate the computer's move
        bCalculating = true;
        _beginthread(Comp,0,(void *)NULL);
      }
      // Update the board display
      DisplayBoard(&Board,hx,hy);
      // Other heartbeat stuff?
      // .....
      bSkipTimer = false;
      break;
    case WM_KEYUP:
      break;
    case WM_COMMAND:
      wmId    = LOWORD(wParam); 
      wmEvent = HIWORD(wParam); 
      // Parse the menu selections:
      switch (wmId) {
        case IDM_FILE_NEW:
          // Reset game
          ResetBoard(&Board);
          PossibleMoves = GenerateMoves(&Board);
          DisplayBoard(&Board,hx,hy);
          bGameOver = false;
          // Deactivate undo/redo menus
          EnableMenuItem(g_hMenu,IDM_EDIT_UNDO,MF_GRAYED);
          EnableMenuItem(g_hMenu,IDM_EDIT_REDO,MF_GRAYED);
          iHistoryPos = iHistoryMax = 0;
          break;
        case IDM_FILE_LOAD:
          if (LoadGame() == true) {
            PossibleMoves = GenerateMoves(&Board);
            DisplayBoard(&Board,hx,hy);
            // Deactivate undo/redo menus
            if (iHistoryPos > 0) EnableMenuItem(g_hMenu,IDM_EDIT_UNDO,MF_ENABLED);
            else EnableMenuItem(g_hMenu,IDM_EDIT_UNDO,MF_GRAYED);
            EnableMenuItem(g_hMenu,IDM_EDIT_REDO,MF_GRAYED);
            bGameOver = false;
          }
          break;
        case IDM_FILE_SAVE:
          SaveGame();
          PossibleMoves = GenerateMoves(&Board);
          DisplayBoard(&Board,hx,hy);
          break;
        case IDM_HELP_ABOUT:
          MessageBox(hWndMain,"Col's Othello v1.0 by Colin Frayn, 2008\ncolin@frayn.net\nhttp://www.frayn.net","About Col's Othello",MB_OK);
          break;
        case IDM_EDIT_UNDO:
          if (iHistoryPos > 0) {
            // Retract the last move
            iHistoryPos--;
            bGameOver = false;

            // Set the board
            UndoMove(&Board,History[iHistoryPos],Undo[iHistoryPos]);

            // Display the board
            PossibleMoves = GenerateMoves(&Board);
            DisplayBoard(&Board,hx,hy);
            EnableMenuItem(g_hMenu,IDM_EDIT_REDO,MF_ENABLED);
          }
          if (iHistoryPos == 0) {
            // Deactivate 'undo' menu
            EnableMenuItem(g_hMenu,IDM_EDIT_UNDO,MF_GRAYED);
          }
          break;
        case IDM_EDIT_REDO:
          if (iHistoryPos < iHistoryMax) {
            // Set the board
            (void)DoMove(&Board,History[iHistoryPos++]);

            // Display the board
            PossibleMoves = GenerateMoves(&Board);
            DisplayBoard(&Board,hx,hy);
            EnableMenuItem(g_hMenu,IDM_EDIT_UNDO,MF_ENABLED);
          }
          if (iHistoryPos == iHistoryMax) {
            // Deactivate 'redo' menu
            EnableMenuItem(g_hMenu,IDM_EDIT_REDO,MF_GRAYED);
          }
          break;
        // Toggle viewing of possible moves on/off
        case IDM_OPTIONS_POSSIBLE:
          if (bShowPossible) {
            bShowPossible = false;
            CheckMenuItem(g_hMenu,IDM_OPTIONS_POSSIBLE,MF_UNCHECKED);
          }
          else {
            bShowPossible = true;
            CheckMenuItem(g_hMenu,IDM_OPTIONS_POSSIBLE,MF_CHECKED);
          }
          break;
        // Toggle 'last move flicker' on/off
        case IDM_OPTIONS_FLICKER:
          if (bFlicker) {
            bFlicker = false;
            CheckMenuItem(g_hMenu,IDM_OPTIONS_FLICKER,MF_UNCHECKED);
          }
          else {
            bFlicker = true;
            CheckMenuItem(g_hMenu,IDM_OPTIONS_FLICKER,MF_CHECKED);
          }
          break;
      // Toggle computer playing white on/off
        case IDM_OPTIONS_COMPUTERWHITE:
          if (bComputer[1]) {
            bComputer[1] = false;
            CheckMenuItem(g_hMenu,IDM_OPTIONS_COMPUTERWHITE,MF_UNCHECKED);
          }
          else {
            bComputer[1] = true;
            CheckMenuItem(g_hMenu,IDM_OPTIONS_COMPUTERWHITE,MF_CHECKED);
          }
          break;
        case IDM_OPTIONS_COMPUTERBLACK:
          if (bComputer[0]) {
            bComputer[0] = false;
            CheckMenuItem(g_hMenu,IDM_OPTIONS_COMPUTERBLACK,MF_UNCHECKED);
          }
          else {
            bComputer[0] = true;
            CheckMenuItem(g_hMenu,IDM_OPTIONS_COMPUTERBLACK,MF_CHECKED);
          }
          break;
        // Perft option
        case IDM_OPTIONS_PERFT:
          if (!bCalculating) {
            bCalculating = true;
            _beginthread(Perft,0,(void *)(&Board));
          }
          break;
        // Force move
        case IDM_OPTIONS_FORCE:
          if (bCalculating) bAbortFlag = true;
          break;
        // Search depth
        case IDM_DEPTH_2:
        case IDM_DEPTH_3:
        case IDM_DEPTH_4:
        case IDM_DEPTH_5:
        case IDM_DEPTH_6:
        case IDM_DEPTH_7:
        case IDM_DEPTH_8:
        case IDM_DEPTH_9:
        case IDM_DEPTH_10:
          if (MaxDepth == 2)  CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_2,MF_UNCHECKED);
          if (MaxDepth == 3)  CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_3,MF_UNCHECKED);
          if (MaxDepth == 4)  CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_4,MF_UNCHECKED);
          if (MaxDepth == 5)  CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_5,MF_UNCHECKED);
          if (MaxDepth == 6)  CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_6,MF_UNCHECKED);
          if (MaxDepth == 7)  CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_7,MF_UNCHECKED);
          if (MaxDepth == 8)  CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_8,MF_UNCHECKED);
          if (MaxDepth == 9)  CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_9,MF_UNCHECKED);
          if (MaxDepth == 10) CheckMenuItem(GetMenu(hWnd),IDM_DEPTH_10,MF_UNCHECKED);
          if (wmId == IDM_DEPTH_2)  MaxDepth = 2; 
          if (wmId == IDM_DEPTH_3)  MaxDepth = 3; 
          if (wmId == IDM_DEPTH_4)  MaxDepth = 4; 
          if (wmId == IDM_DEPTH_5)  MaxDepth = 5; 
          if (wmId == IDM_DEPTH_6)  MaxDepth = 6; 
          if (wmId == IDM_DEPTH_7)  MaxDepth = 7; 
          if (wmId == IDM_DEPTH_8)  MaxDepth = 8; 
          if (wmId == IDM_DEPTH_9)  MaxDepth = 9; 
          if (wmId == IDM_DEPTH_10) MaxDepth = 10; 
          CheckMenuItem(GetMenu(hWnd),wmId,MF_CHECKED);
          break;
        // Search time
        case IDM_TIME_1S:
        case IDM_TIME_2S:
        case IDM_TIME_5S:
        case IDM_TIME_10S:
        case IDM_TIME_30S:
        case IDM_TIME_1M:
        case IDM_TIME_INFINITE:
          if (SearchTime == 1) CheckMenuItem(GetMenu(hWnd),IDM_TIME_1S,MF_UNCHECKED);
          if (SearchTime == 2) CheckMenuItem(GetMenu(hWnd),IDM_TIME_2S,MF_UNCHECKED);
          if (SearchTime == 5) CheckMenuItem(GetMenu(hWnd),IDM_TIME_5S,MF_UNCHECKED);
          if (SearchTime == 10) CheckMenuItem(GetMenu(hWnd),IDM_TIME_10S,MF_UNCHECKED);
          if (SearchTime == 30) CheckMenuItem(GetMenu(hWnd),IDM_TIME_30S,MF_UNCHECKED);
          if (SearchTime == 60) CheckMenuItem(GetMenu(hWnd),IDM_TIME_1M,MF_UNCHECKED);
          if (SearchTime == 1000000) CheckMenuItem(GetMenu(hWnd),IDM_TIME_INFINITE,MF_UNCHECKED);
          if (wmId == IDM_TIME_1S)  SearchTime = 1; 
          if (wmId == IDM_TIME_2S)  SearchTime = 2; 
          if (wmId == IDM_TIME_5S)  SearchTime = 5; 
          if (wmId == IDM_TIME_10S)  SearchTime = 10; 
          if (wmId == IDM_TIME_30S)  SearchTime = 30; 
          if (wmId == IDM_TIME_1M)  SearchTime = 60; 
          if (wmId == IDM_TIME_INFINITE)  SearchTime = 1000000; 
          CheckMenuItem(GetMenu(hWnd),wmId,MF_CHECKED);
          break;
        // Quit the programme
        case IDM_FILE_EXIT:
        case IDOK:
        case IDCANCEL:
          if (MessageBox(hWndMain, "Really Quit?", "Quit Program", MB_YESNO) == IDYES) {
            PostQuitMessage(IDCANCEL);
          }
          return TRUE;
        default:
          return DefWindowProc(hWnd, message, wParam, lParam);
      }
      break;
    case WM_PAINT:
      hDC = BeginPaint(hWnd, &ps);
      ReleaseDC(hWnd, hDC);
      DisplayBoard(&Board,hx,hy);
      break;
    case WM_DESTROY:
      PostQuitMessage(IDCANCEL);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return FALSE;
}

// Set up the main window
void SetupMainWindow(void) {
  WNDCLASSEX wcex;
  char szWindow[] = "Main Window";
  ATOM at;
  
  // Setup the window class & register it
	wcex.style       = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra  = 0;
	wcex.cbWndExtra  = 0;
	wcex.hInstance   = g_hInstance;
	wcex.hIcon       = NULL;
	wcex.hCursor     = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	 = (LPCSTR)IDR_MENU;
	wcex.lpszClassName = szWindow;
	wcex.hIconSm       = NULL;
  wcex.cbSize        = sizeof(WNDCLASSEX); 
    
  at = RegisterClassEx(&wcex);
    
  hWndMain = CreateWindow(szWindow,"Col's Othello v1.0    (c) 2008 Colin Frayn   colin@frayn.net", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                          CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, g_hInstance, NULL);

  EnableWindow(hWndMain,true);
  ShowWindow(hWndMain,SW_SHOW);
  UpdateWindow(hWndMain);

  // Get a handle to the main menu
  g_hMenu = GetMenu(hWndMain);
}

// Setup the GUI elements
void SetupViewport(HWND hDlg) {
  RECT Rect;

   // Get the box dimensions
  GetClientRect(hDlg,&Rect);
  WindowWidth = Rect.right - Rect.left;
  WindowHeight = Rect.bottom - Rect.top;

  // Set up a few other variables  
  glShadeModel(GL_FLAT);
  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(true);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  // Setup the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, (float)WindowWidth/(float)WindowHeight, 1.0f, 30000.0f);    
  glViewport( 0, 0, WindowWidth, WindowHeight);
}


// Set up the window ready for OpenGL commands
HGLRC Graphics_SetupWindow(HWND hWnd, HDC hDC) {
  PIXELFORMATDESCRIPTOR pfd;
  HGLRC hRC;
  int pixelFormat;

  // Set OpenGL rendering stuff
  memset(&pfd, 0,sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 32;
  pfd.iLayerType = PFD_MAIN_PLANE;
  pfd.cStencilBits = 8;
  
  // Set it up...
  pixelFormat = ChoosePixelFormat(hDC, &pfd);
  if (pixelFormat == 0) {
    MessageBox(hWndMain,"Could not choose Pixel format","Pixel format problem",MB_OK);
    return NULL;
  }
  if (!SetPixelFormat (hDC, pixelFormat, &pfd)) {
    MessageBox(hWndMain,"Could not set Pixel format","Pixel format problem",MB_OK);
    return NULL;
  }
  hRC = wglCreateContext(hDC);
  if (!hRC) {
    MessageBox(hWndMain,"Could not create HGLRC","HGLRC problem",MB_OK);
  }

  // Make this context the current one
  wglMakeCurrent(hDC, hRC);

  // Setup the fonts
  if(!Graphics_SetupFonts(hDC,hRC,10)) {
		MessageBox(hWndMain,"Failed to open font", "Font error!", MB_OK);
		exit(1);
	}

  // Quadrics style
  Graphics_quadObj = gluNewQuadric();
  gluQuadricDrawStyle(Graphics_quadObj, GLU_FILL);
  gluQuadricNormals(Graphics_quadObj, GLU_SMOOTH);

  // Set up default settings
  glShadeModel(GL_SMOOTH);
  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
  glDepthMask(true);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glDisable(GL_STENCIL_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_BLEND);
  SetupLighting();

  return hRC;
}


// Setup the glyphs etc. for the OpenGL Fonts
bool Graphics_SetupFonts(HDC hDC, HGLRC hRC, int size) {
  int base;

  wglMakeCurrent(hDC,hRC);
  // Setup storage for 256 characters in our display list
  base = glGenLists(256);
  g_iBase = base;
  // Create and select a new font
  HFONT font = CreateFont(size,0,0,0,FW_BOLD,false,false,false,ANSI_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,
                    ANTIALIASED_QUALITY,FF_DONTCARE|DEFAULT_PITCH,NULL);
	SelectObject(hDC, font);

  // Setup the display list
  if (wglUseFontOutlines(hDC,0,255,base,0.0f,0.1f,WGL_FONT_POLYGONS,Graphics_gmf)) {
    return true;
  }
  MessageBox(hWndMain,"Could not set up OpenGL font outlines","Font setup error",MB_OK);
  return false;
}


// Display the board
void DisplayBoard(BoardType *B, int hx, int hy) {
  float xstep, ystep;

  flicker += 0.8f;
  if (!bFlicker || flicker > PIF*8.5f) flicker = PIF*8.5f;
  // Select the projection matrix and reset it
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // Select the modelview matrix and reset it
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Reset the window
  glClearColor(0.0f, 0.0f, 0.0f, 0.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  // Draw text labels
  glTranslatef(0.0f,0.9f,0.0f);
  if (B->side == WHITE) { glColor4f(1.0f,1.0f,1.0f,1.0f); Graphics_PrintString("White",0.12f,0.12f); }
  else { glColor4f(0.3f,0.3f,0.3f,1.0f); Graphics_PrintString("Black",0.12f,0.12f); }
  if (g_Depth > 0) {
    glColor4f(0.0f,1.0f,1.0f,1.0f);
    glLoadIdentity();
    glTranslatef(-0.7f,0.9f,0.0f);
    char strTemp[20];
    sprintf(strTemp,"%d ply",g_Depth);
    Graphics_PrintString(strTemp,0.1f,0.1f);
    glLoadIdentity();
    glTranslatef(0.7f,0.9f,0.0f);
    sprintf(strTemp,"%.2f sec",(float)GetElapsedTime()/100.0f);
    Graphics_PrintString(strTemp,0.1f,0.1f);
  }
  glColor4f(1.0f,1.0f,1.0f,1.0f);
  glLoadIdentity();

  // Draw the board
  glBegin(GL_LINES);
  xstep = (2.0f - (BorderX*2.0f)) / 8.0f;
  for (int x=0;x<=8;x++) {
    glVertex3f(-1.0f+BorderX+((float)x*xstep),-1.0f+BorderY,0.0f);
    glVertex3f(-1.0f+BorderX+((float)x*xstep), 1.0f-(BorderY+TopBorder),0.0f);
  }
  ystep = (2.0f - (BorderY*2.0f + TopBorder)) / 8.0f;
  for (int y=0;y<=8;y++) {
    glVertex3f(-1.0f+BorderX,-1.0f+BorderY+((float)y*ystep),0.0f);
    glVertex3f( 1.0f-BorderX,-1.0f+BorderY+((float)y*ystep),0.0f);
  }
  glEnd();

  // Draw the counters
  glTranslatef(-(xstep*4.5f),(ystep*3.5f)-(TopBorder/2.0f),0.0f);
  for (int y=0; y<8; y++) {
    for (int x=0; x<8; x++) {
      int piece = 0;
      if (B->white & Mask[y*8+x]) piece = WHITE;
      if (B->black & Mask[y*8+x]) piece = BLACK;
      // Draw the counter
      glTranslatef(xstep,0.0f,0.0f);
      if (piece) {
        // Setup the colour: White for white, dark grey for black
        float col = 0.65f+((float)piece*0.35f);
        if (iHistoryPos > 0 && History[iHistoryPos-1] == (y*8)+x) col *= (2.0f + (float)sin(flicker)) / 3.0f;
        glColor4f(col,col,col,1.0f);
        // Draw the counter
        gluSphere(Graphics_quadObj, 0.1f, 30, 40);
      }
      else {
        // See if this is a possible move
        if (bShowPossible && (PossibleMoves & Mask[y*8 + x])) {
          // Setup the colour: White for white, dark grey for black. Add transparency
          glColor4f(0.65f+((float)piece*0.35f),0.65f+((float)piece*0.35f),0.65f+((float)piece*0.35f),0.3f);
          // Draw the ghost counter
          glEnable(GL_BLEND);
          glDisable(GL_LIGHTING);
          gluSphere(Graphics_quadObj, 0.05f, 30, 40);
          glEnable(GL_LIGHTING);
          glDisable(GL_BLEND);
        }
        if (g_ThinkingMove == (y*8)+x) {
          glColor4f(1.0f,1.0f,0.0f,1.0f);
          Graphics_PrintString("?",0.25f,0.25f);
        }
        else if (g_BestMove == (y*8)+x) {
          glColor4f(0.0f,1.0f,0.0f,1.0f);
          glBegin(GL_TRIANGLES);
          glVertex3f(-xstep*0.2f,0.0f,-0.1f);
          glVertex3f(xstep*0.1f,-ystep*0.4f,-0.1f);
          glVertex3f(0.0f,-ystep*0.15f,-0.1f);
          glVertex3f(xstep*0.1f,-ystep*0.4f,-0.1f);
          glVertex3f(0.0f,-ystep*0.15f,-0.1f);
          glVertex3f(xstep*0.3f,ystep*0.4f,-0.1f);
          glEnd();
        }
      }
    }
    glTranslatef(-(xstep*8.0f),-ystep,0.0f);
  }

  // Mouse hover
  if (hx>=0 && hx < 8 && hy >= 0 && hy < 8) {
    glLoadIdentity();
    glTranslatef((float)(hx-4)*xstep,(float)(4-hy)*ystep - TopBorder/2.0f,0.05f);
    if (PossibleMoves & Mask[hy*8 + hx]) glColor4f(0.0f,0.5f,0.0f,1.0f);
    else glColor4f(0.0f,0.2f,0.0f,1.0f);
    glBegin(GL_QUADS);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(xstep,0.0f,0.0f);
    glVertex3f(xstep,-ystep,0.0f);
    glVertex3f(0.0f,-ystep,0.0f);
    glEnd();
  }

  // Swap the buffers (making the animation smooth)
  glFlush();
  SwapBuffers(BoardDC);
}

#define Graphics_AntiAliasSize (4)

// Custom GL print routine. Print the specified text, at the current position
// (with the specified alignment, defaulting to Left, Bottom corner)
void Graphics_PrintString(string strText, float scalex, float scaley) {
	float length = 0.0f;	// Used To Find The Length Of The Text
  float height = 0.0f;
  float shufflex, shuffley;

  // Set shuffle to 1 pixel
  shufflex = 1.0f * Graphics_AntiAliasSize / (float)WindowWidth;
  shuffley = 1.0f * Graphics_AntiAliasSize / (float)WindowHeight;

  // Check we don't have a dud string
  if (strText.length() == 0) return;

  // Make sure we're plotting solid polygons
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glPushMatrix();

  // Get text length & maximum height
  for (unsigned int loop=0;loop<strText.length();loop++) {
		length += Graphics_gmf[strText[loop]].gmfCellIncX;
    if (Graphics_gmf[strText[loop]].gmfBlackBoxY > height) height = Graphics_gmf[strText[loop]].gmfBlackBoxY;
	}
  length *= scalex;
  height *= scaley;

  // Text alignment
  glTranslatef(-length/2.0f,0.0f,0.0f);
  glTranslatef(0.0f,-height/2.0f,0.0f);

  // Set the text base, scale
  glScalef(scalex, scaley, 1.0f);
	glListBase(g_iBase);

  // Draw the main text
  float col[4];
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glGetFloatv(GL_CURRENT_COLOR,col);
  col[3] /= 3.0f;
  glColor4fv(col);
  glPushMatrix();
  glTranslatef(-shufflex,0.0f,0.0f);
  glCallLists((int)strText.length(), GL_UNSIGNED_BYTE, strText.c_str());
  glPopMatrix();
  glPushMatrix();
  glTranslatef(shufflex,0.0f,0.0f);
  glCallLists((int)strText.length(), GL_UNSIGNED_BYTE, strText.c_str());
  glPopMatrix();
  glPushMatrix();
  glTranslatef(0.0f,-shuffley,0.0f);
  glCallLists((int)strText.length(), GL_UNSIGNED_BYTE, strText.c_str());
  glPopMatrix();
  glPushMatrix();
  glTranslatef(0.0f,shuffley,0.0f);
  glCallLists((int)strText.length(), GL_UNSIGNED_BYTE, strText.c_str());
  glPopMatrix();
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  col[3] *= 3.0f;
  glColor4fv(col);
  glFrontFace(GL_CCW); // Call Lists screws with this value, so fix it (not my bug!)
  glPopMatrix();
}


// Set up the lighting parameters
void SetupLighting(void) {
  GLfloat MaterialSpecular[] = {0.4f, 0.4f, 0.4f, 1.0f};
  GLfloat LightPosition[] = {4.0f, 4.0f, -10.0f, 1.0f};

  // Set lighting parameters
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);

  // Set main light
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT0);

  // Material properties
//  glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);
  glMaterialf(GL_FRONT, GL_SHININESS, 64.0f);
}

// Deal with a left mouse click
void MouseClick(BoardType *B, int hx, int hy) {
  int sq;

  // Ignore if it's not our turn
  if (bCalculating || bComputer[(B->side+1)/2]) return;

  // Check this square
  if (hx<0 || hx>=8 || hy<0 || hy>=8) return;
  if ((B->white|B->black)&Mask[hy*8+hx]) return;

  // Get a list of all legal moves
  PossibleMoves = GenerateMoves(&Board);

  // Is this a legal move?
  sq = hy*8 + hx;
  if (!(PossibleMoves & Mask[sq])) return;
  
  // Do the move
  Undo[iHistoryPos] = DoMove(B,sq);
  History[iHistoryPos++] = sq;
  flicker = 0.0f;
  iHistoryMax = iHistoryPos;
  PossibleMoves = GenerateMoves(&Board);
  EnableMenuItem(g_hMenu,IDM_EDIT_UNDO,MF_ENABLED);
  EnableMenuItem(g_hMenu,IDM_EDIT_REDO,MF_GRAYED);
}

// Game over
void GameOver(void) {
  int countwhite = 0, countblack = 0;
  char strTemp[64];

  countwhite = Count(Board.white);
  countblack = Count(Board.black);

  if (countwhite >  countblack) sprintf(strTemp,"White wins %d vs. %d",countwhite, countblack);
  if (countwhite <  countblack) sprintf(strTemp,"Black wins %d vs. %d",countblack, countwhite);
  if (countwhite == countblack) sprintf(strTemp,"Game is a draw");

  // Display the results
  MessageBox(hWndMain,strTemp,"Game Over",MB_OK);
  bGameOver = true;
}