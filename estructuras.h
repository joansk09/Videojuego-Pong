struct pos{
	int x;
	int y;
};
typedef struct pos POS;

struct campo{
	POS pos;
	int tipo;
};
typedef struct campo CAMPO;

struct player{
	POS pos;
};
typedef struct player JUGADOR;

struct pelota{
	POS pos;
	int estado;
};
typedef struct pelota PELOTA;