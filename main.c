#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include "estructuras.h"

#define BLOCKCAMP 	96
#define TAMGAME 	18
#define TAMJUG		5

#define HORIZONTAL		1
#define LATERAL			2

#define IZQ			1
#define DER			2

#define IDT_TIMER1 1

#define GOLP1 1
#define GOLP2 2

void crearCampo(RECT);
void DibujarCampo(HDC);
void DibujarJugador(HDC hdc);
void NuevoJugador(RECT);
int	MoverPelota(RECT, HDC);

int MoverJugador(int);
void MoverIA(RECT, int);

RECT rectP1,	rectP2,		pelotaRt;

int IAdir = DER;

CAMPO *campoG = NULL;
JUGADOR *jugador1 = NULL;
JUGADOR *jugador2 = NULL;
PELOTA balon;

int pelotaDx = 1;
int pelotaDy = -1;

int GOL = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC  hdc;
	RECT rect;
	JUGADOR *jugador = NULL;
	static int cuentaIA = 0;
	static int cuentaPel = 0;
	
	switch(Message) {
		case WM_CREATE:{
			SetTimer(hwnd, IDT_TIMER1, 20, NULL);
			GetClientRect(hwnd,&rect);
			NuevoJugador(rect);
			crearCampo(rect);
			InvalidateRect(hwnd,&pelotaRt,TRUE);
			break;
		}
		
		case WM_TIMER:{
			switch(wParam){
				case IDT_TIMER1:{
					GetClientRect(hwnd,&rect);
					cuentaIA++;
					cuentaPel++;
					InvalidateRect(hwnd,&pelotaRt,TRUE);
					InvalidateRect(hwnd,&rectP1,TRUE);
					InvalidateRect(hwnd,&rectP2,TRUE);
					if(cuentaPel==4){
						MoverPelota(rect,hdc);
						cuentaPel=1;
					}
				}
			}
			break;
		}
		case WM_KEYDOWN:{
			GetClientRect(hwnd,&rect);
			int TYPE = (wParam==VK_LEFT)? IZQ : (wParam==VK_RIGHT)? DER : 0;
			int TYPE2;
			if(GetAsyncKeyState(0x41) & 0x8000){
				TYPE2 = IZQ;
			}
			else if(GetAsyncKeyState(0x44) & 0x8000){
    			TYPE2 = DER;
			}
			else{
				TYPE2 = 0;
			}
			MoverJugador(TYPE);
			MoverIA(rect, TYPE2);
			break;
		}
		
		case WM_PAINT:{
			hdc = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd, &rect);
			DibujarCampo(hdc);
			DibujarJugador(hdc);
			break;
		}
		
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG msg; /* A temporary location for all messages */

	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+2);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Caption",WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		440, /* width */
		580, /* height */
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}

void crearCampo(RECT rect){
	int i=0,aux=0;
	campoG = (CAMPO *) malloc(sizeof(CAMPO)*BLOCKCAMP);
	campoG[0].pos.x = 1;
	campoG[0].pos.y = 1;
	campoG[0].tipo = HORIZONTAL;
	for(i=1; i<21; i++){
		campoG[i].pos.x = campoG[i-1].pos.x + 1;
		campoG[i].pos.y = 1;
		campoG[i].tipo = HORIZONTAL;
	}
	for(i=21; i<42; i++){
		campoG[i].pos.x = campoG[0].pos.x + (i-21);
		campoG[i].pos.y = rect.bottom / TAMGAME - 1;
		campoG[0].tipo = HORIZONTAL;
	}
	for(i=42; i<68; i++){
		campoG[i].pos.x = campoG[0].pos.x;
		campoG[i].pos.y = campoG[0].pos.y + (i-41);
		campoG[0].tipo = LATERAL;
	}
	for(i=68; i<96; i++){
		campoG[i].pos.x = rect.right / TAMGAME - 1;
		campoG[i].pos.y = campoG[0].pos.y + (i-68);
		campoG[0].tipo = LATERAL;
	}
	balon.pos.x = rect.right / TAMGAME /2;
	balon.pos.y = rect.bottom / TAMGAME /2;

	rectP1.left = campoG[47].pos.x * TAMGAME;
	rectP1.top  = campoG[47].pos.y * TAMGAME;
	rectP1.right = campoG[71].pos.x * TAMGAME;
	rectP1.bottom =	campoG[71].pos.y * TAMGAME - TAMGAME;

	rectP2.left = campoG[66].pos.x * TAMGAME;
	rectP2.top = campoG[66].pos.y * TAMGAME + 1;
	rectP2.right = campoG[91].pos.x * TAMGAME;
	rectP2.bottom = campoG[91].pos.y * TAMGAME - TAMGAME;
	
	pelotaRt.left = balon.pos.x * TAMGAME;
	pelotaRt.top = balon.pos.y * TAMGAME;
	pelotaRt.right = balon.pos.x * TAMGAME + TAMGAME;
	pelotaRt.bottom = balon.pos.y * TAMGAME + TAMGAME;
}

void DibujarCampo(HDC hdc){
	int i;
	SetDCBrushColor(hdc, RGB(112, 128, 144));
	HBRUSH hBrush = CreateSolidBrush(GetDCBrushColor(hdc));
	HBRUSH hOldBrush;
	hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	
	for(i=0; i<BLOCKCAMP; i++){
		RoundRect(hdc, campoG[i].pos.x * TAMGAME, campoG[i].pos.y * TAMGAME, 
			    		campoG[i].pos.x * TAMGAME + TAMGAME, campoG[i].pos.y * TAMGAME + TAMGAME, 5, 5);	
	}
	SelectObject(hdc, hOldBrush);
	DeleteObject(hBrush);

	SetDCBrushColor(hdc, RGB(46, 139, 87));
	HBRUSH pincelPelota = CreateSolidBrush(GetDCBrushColor(hdc));
	HBRUSH pinBalon = (HBRUSH)SelectObject(hdc, pincelPelota);
	
	Ellipse(hdc, balon.pos.x * TAMGAME, balon.pos.y * TAMGAME, 
				 balon.pos.x * TAMGAME + TAMGAME, balon.pos.y * TAMGAME + TAMGAME);
	SelectObject(hdc, pinBalon);
	DeleteObject(pincelPelota);
}

void NuevoJugador(RECT rect){
	int i = 0;
	jugador1 = (JUGADOR *) malloc(sizeof(JUGADOR) * TAMJUG);
	jugador2 = (JUGADOR *) malloc(sizeof(JUGADOR) * TAMJUG);
	if(jugador1 == NULL){
		MessageBox(NULL,"Sin memoria","Error",MB_ICONERROR);
	}
	jugador1[0].pos.x = 7;
	jugador1[0].pos.y = 5;
	for(i=0; i<TAMJUG; i++){
		jugador1[i].pos.x = jugador1[0].pos.x + i;
		jugador1[i].pos.y = jugador1[0].pos.y;
	}
	for(i=0; i<TAMJUG; i++){
		jugador2[i].pos.x = jugador1[i].pos.x;
		jugador2[i].pos.y = jugador1[i].pos.y + 21;
	}
}

void DibujarJugador(HDC hdc){
	int i;
	POINT jug1[4]	,	jug2[4];
	
	SetDCBrushColor(hdc, RGB(255, 50, 0));
	HBRUSH hBrush = CreateSolidBrush(GetDCBrushColor(hdc));
	HBRUSH hOldBrush;
	
	hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	jug1[0].x = jugador1[0].pos.x * TAMGAME;						jug1[0].y = jugador1[0].pos.y * TAMGAME;
	jug1[1].x = jugador1[TAMJUG-1].pos.x * TAMGAME + TAMGAME;		jug1[1].y = jugador1[TAMJUG-1].pos.y * TAMGAME;
	jug1[2].x = jugador1[TAMJUG-1].pos.x * TAMGAME + TAMGAME;		jug1[2].y = jugador1[TAMJUG-1].pos.y * TAMGAME - TAMGAME;
	jug1[3].x = jugador1[0].pos.x * TAMGAME;						jug1[3].y = jugador1[0].pos.y * TAMGAME - TAMGAME;
	Polygon(hdc, jug1, 4);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hBrush);
	
	SetDCBrushColor(hdc, RGB(0, 0, 0));
	HBRUSH jug2X = CreateSolidBrush(GetDCBrushColor(hdc));
	HBRUSH jug2AX;
	jug2AX = (HBRUSH)SelectObject(hdc, jug2X);
	jug2[0].x = jugador2[0].pos.x * TAMGAME;						jug2[0].y = jugador2[0].pos.y * TAMGAME;
	jug2[1].x = jugador2[TAMJUG-1].pos.x * TAMGAME + TAMGAME;		jug2[1].y = jugador2[TAMJUG-1].pos.y * TAMGAME;
	jug2[2].x = jugador2[TAMJUG-1].pos.x * TAMGAME + TAMGAME;		jug2[2].y = jugador2[TAMJUG-1].pos.y * TAMGAME - TAMGAME;
	jug2[3].x = jugador2[0].pos.x * TAMGAME;						jug2[3].y = jugador2[0].pos.y * TAMGAME - TAMGAME;
	Polygon(hdc, jug2, 4);	
	SelectObject(hdc, jug2AX);
	DeleteObject(jug2X);
}

int MoverJugador(int MovType){
	int i=0,j=0;
	if(MovType == IZQ){
		if(jugador1[0].pos.x != campoG[47].pos.x + 1){
			for(i=0; i<TAMJUG; i++)
				jugador1[i].pos.x -= 1;
		}
	}
	else if(MovType == DER){
		if(jugador1[TAMJUG-1].pos.x != campoG[72].pos.x - 1){
			for(i=0; i<TAMJUG; i++)
				jugador1[i].pos.x += 1;
		}
	}
}

void MoverIA(RECT rect, int MovType){
	/* MOVIMIENTO DE IA
	int i=0;
	if(IAdir == DER){
		if(jugador2[TAMJUG-1].pos.x <= campoG[93].pos.x - 2){
			for(i=0; i<TAMJUG; i++){
				jugador2[i].pos.x += 1;
			}
		}
		else{
			IAdir = IZQ;
		}
	}
	else if(IAdir == IZQ){
		if(jugador2[0].pos.x >= 3){
			for(i=0; i<TAMJUG; i++){
				jugador2[i].pos.x -= 1;
			}			
		}
		else{
			IAdir = DER;
		}
	}
	*/
	int i=0,j=0;
	if(MovType == IZQ){
		if(jugador2[0].pos.x >= 3){
			for(i=0; i<TAMJUG; i++)
				jugador2[i].pos.x -= 1;
		}
	}
	else if(MovType == DER){
		if(jugador2[TAMJUG-1].pos.x <= campoG[93].pos.x - 2){
			for(i=0; i<TAMJUG; i++)
				jugador2[i].pos.x += 1;
		}
	}
	
}

int	MoverPelota(RECT rect, HDC hdc){
	int i = 0;
	srand(time(NULL));
	balon.pos.x += pelotaDx;
	balon.pos.y += pelotaDy;
	
	if(balon.pos.x <= campoG[0].pos.x + 1){
		pelotaDx = 1;
	}
	if(balon.pos.x >= campoG[92].pos.x - 1){
		pelotaDx = -1;
	}
	
	if(balon.pos.y <= campoG[43].pos.y - 2){
		balon.pos.x = rect.right / TAMGAME /2;
		balon.pos.y = rect.bottom / TAMGAME /2;
		pelotaDx = 1;
		pelotaDy = -1;
		PlaySound(TEXT("gol.wav"), NULL, SND_FILENAME | SND_ASYNC);
		Sleep(1000);
	}
	if(balon.pos.y >= campoG[67].pos.y + 1){
		balon.pos.x = rect.right / TAMGAME /2;
		balon.pos.y = rect.bottom / TAMGAME /2;
		pelotaDx = 1;
		pelotaDy = -1;
		PlaySound(TEXT("gol.wav"), NULL, SND_FILENAME | SND_ASYNC);
		Sleep(1000);
	}
	if(balon.pos.x >= jugador1[0].pos.x && balon.pos.x <= jugador1[TAMJUG-1].pos.x
		&&	balon.pos.y >= jugador1[0].pos.y && balon.pos.y <= jugador1[TAMJUG-1].pos.y	){
		while(i<TAMJUG){
			if(balon.pos.x == jugador1[i].pos.x && balon.pos.y == jugador1[i].pos.y){
				pelotaDx = rand()% 7;
				pelotaDx = (pelotaDx == 0)?	-3 : (pelotaDx==1)? -2 : (pelotaDx==2)? -1 : (pelotaDx==3)? 0 : (pelotaDx==4)? 1 : (pelotaDx==5)? 2 : 3;
				pelotaDy = 0;
			}
			i++;
		}
		pelotaDy = 1;
	}
	if(balon.pos.x >= jugador2[0].pos.x && balon.pos.x <= jugador2[TAMJUG-1].pos.x
		&&	balon.pos.y >= jugador2[0].pos.y - 1 && balon.pos.y <= jugador2[TAMJUG-1].pos.y - 1	){
		pelotaDx = rand()% 7;
		pelotaDx = (pelotaDx == 0)?	-3 : (pelotaDx==1)? -2 : (pelotaDx==2)? -1 : (pelotaDx==3)? 0 : (pelotaDx==4)? 1 : (pelotaDx==5)? 2 : 3;
		pelotaDy = -1;
	}
	pelotaRt.left = balon.pos.x * TAMGAME;
	pelotaRt.top = balon.pos.y * TAMGAME;
	pelotaRt.right = balon.pos.x * TAMGAME + TAMGAME;
	pelotaRt.bottom = balon.pos.y * TAMGAME + TAMGAME;
}
