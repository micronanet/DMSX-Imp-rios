// =====================================================================
// estado.h — IMPÉRIOS: o estado do jogo (mapa, unidades, cidades, povos)
// =====================================================================
#pragma once

// o tamanho do mapa escolhe-se no início do jogo (a memória reserva-se para o maior)
const int MAPA_W_MAX = 112, MAPA_H_MAX = 70, MAPA_N_MAX = MAPA_W_MAX * MAPA_H_MAX;
int MAPA_W = 80, MAPA_H = 50, MAPA_N = 80 * 50;
// opções do mundo (como no original, mais as ilhas e os recursos)
struct OpcoesMundo { uint8_t tamanho, terra, ilhas, clima, temperatura, idade, recursos; };
OpcoesMundo mundo = { 1, 1, 1, 1, 1, 1, 1 };
const uint8_t TAM_W[4] = { 64, 80, 96, 112 }, TAM_H[4] = { 40, 50, 60, 70 };
void poeTamanhoMapa(int w, int h) { MAPA_W = w; MAPA_H = h; MAPA_N = w * h; }
const int MAX_UNIDADES = 384;
const int MAX_CIDADES  = 96;
const int N_CIVS       = 8;          // 0 = bárbaros, 1..7 = povos
const int SEM          = 255;        // "nenhum" nos campos de 8 bits

// ---------------- mapa ----------------
// ter: bits 0-3 terreno, bit 4 recurso especial, bit 5 aldeia tribal
// melh: melhoramentos (M_ESTRADA, M_FERROVIA, M_REGA, M_MINA, M_FORTALEZA, M_POLUICAO)
// vis: bit c = o povo c já viu esta casa
// cont: número do continente (0 = mar)
uint8_t * mapaTer  = nullptr;
uint8_t * mapaMelh = nullptr;
uint8_t * mapaVis  = nullptr;
uint8_t * mapaCont = nullptr;
const uint8_t B_ESPECIAL = 16, B_ALDEIA = 32;

// ---------------- unidades ----------------
enum EstadoUnidade : uint8_t { EU_NADA, EU_FORTIFICANDO, EU_FORTIFICADA, EU_SENTINELA, EU_TRABALHO, EU_DESTINO, EU_A_BORDO };
enum Trabalho : uint8_t { TB_NADA, TB_ESTRADA, TB_REGA, TB_MINA, TB_FORTALEZA, TB_LIMPAR, TB_FERROVIA };

struct Unidade {
  uint8_t viva;
  uint8_t tipo;
  uint8_t civ;
  uint8_t x, y;
  uint8_t mov;          // movimento que falta, em terços de ponto
  uint8_t estado;       // EstadoUnidade
  uint8_t trabalho;     // Trabalho (colonos)
  uint8_t prog;         // progresso do trabalho
  uint8_t vet;          // veterana
  uint8_t casa;         // cidade que a sustenta (SEM = nenhuma)
  uint8_t gx, gy;       // destino (gx = SEM: nenhum)
  uint8_t comb;         // aviões: turnos no ar
  uint8_t ia;           // ordem da inteligência artificial
};
Unidade * U = nullptr;

// ---------------- cidades ----------------
struct Cidade {
  uint8_t  viva;
  char     nome[16];
  uint8_t  civ, civOrig;
  uint8_t  x, y;
  uint8_t  tam;
  int16_t  comida;        // celeiro
  int16_t  escudos;       // produção acumulada
  uint8_t  prod;          // o que está a fazer: 0..N_UNIDADES-1 unidade; PROD_EDIF+e edifício; PROD_MARAV+m maravilha
  uint8_t  especialistas[3]; // artistas, cobradores, cientistas
  uint32_t edif;          // edifícios (bits)
  uint32_t trab;          // casas trabalhadas (bits 0..20, ver RAIO_CIDADE)
  uint8_t  desordem;      // turnos seguidos em desordem
  uint8_t  comprou;       // já comprou este turno
  uint8_t  fundada;       // bit 0: capital original; bit 1 (CID_AUTO): produção automática
  uint8_t  rotaCom;       // rotas de comércio (caravanas), 0..3
};
Cidade * CID = nullptr;
const uint8_t PROD_EDIF = 64, PROD_MARAV = 128;
const uint8_t CID_AUTO = 2;          // em Cidade::fundada: o governador escolhe a produção
const int PROD_AUTO = 254;            // item "Auto" da lista de produção
inline bool producaoAuto(const Cidade & k) { return k.fundada & CID_AUTO; }

// ---------------- povos ----------------
struct Civ {
  uint8_t  viva;
  uint8_t  povo;          // índice em POVO (SEM = bárbaros)
  uint8_t  humano;
  uint8_t  governo;
  uint8_t  anarquia;      // turnos de anarquia que faltam
  uint8_t  govDepois;     // governo a seguir à anarquia
  int32_t  ouro;
  int16_t  lampadas;      // ciência acumulada
  uint8_t  pesquisa;      // tecnologia a pesquisar (SEM_TEC = nenhuma)
  uint8_t  taxa, luxo;    // em dezenas de % (ciência = o resto)
  uint8_t  tecs[(N_TECS + 7) / 8];
  uint8_t  nTecs;
  uint8_t  guerra;        // bit c: em guerra com o povo c
  uint8_t  contacto;      // bit c: já conhece o povo c
  uint8_t  capital;       // cidade do palácio (SEM = nenhuma)
  uint8_t  nave[3];       // peças da nave: estruturas, componentes, módulos
  int16_t  naveChega;     // turno em que a nave chega (0 = não partiu)
  uint8_t  cidadesFundadas;
  uint8_t  atitude;       // inteligência artificial: 0 pacífico .. 4 hostil
  int16_t  pontos;
};
Civ CIV[N_CIVS];
// cenas animadas à espera de ser mostradas (maravilha construída, cidade conquistada)
enum TipoCenaJogo : uint8_t { CN_MARAVILHA, CN_CONQUISTA, CN_DESCOBERTA };
struct CenaPendente { uint8_t tipo, a, b; int16_t c; };
const int MAX_CENAS = 6;
CenaPendente cenasPendentes[MAX_CENAS];
int nCenas = 0;
void pedeCena(uint8_t tipo, int a, int c = 0, int b = 0) { if (nCenas < MAX_CENAS) { CenaPendente & p = cenasPendentes[nCenas++]; p.tipo = tipo; p.a = a; p.b = b; p.c = c; } }
uint8_t encontrosPendentes = 0;   // bit c: falta mostrar o primeiro encontro com o povo c (o líder aparece e oferece a paz)

// ---------------- jogo ----------------
int16_t  turno = 0;
uint8_t  jogador = 1;           // o povo humano
uint8_t  dificuldade = 1;
uint8_t  nCivsJogo = 7;         // povos no início (com o jogador)
int8_t   maravilhaCidade[N_MARAVILHAS];   // cidade que a tem (-1 = ninguém)
uint32_t sementeMapa = 1;
uint8_t  fimDeJogo = 0;          // 0 a jogar; 1 vitória; 2 derrota
uint8_t  tecDescoberta[N_TECS];  // quem descobriu primeiro (para a Grande Biblioteca e pontos)

inline bool temTec(int c, int t)  { return t == SEM_TEC || (CIV[c].tecs[t >> 3] >> (t & 7)) & 1; }
inline void poeTec(int c, int t)  { CIV[c].tecs[t >> 3] |= 1 << (t & 7); }
inline bool temEdif(const Cidade & k, int e) { return (k.edif >> e) & 1; }
inline bool emGuerra(int a, int b) { return a == 0 || b == 0 || ((CIV[a].guerra >> b) & 1); }

// raio da cidade: as 21 casas (5x5 sem os cantos); a 0 é o centro
const int8_t RAIO_DX[21] = { 0,  0, 1, 0, -1,  1, 1, -1, -1,  0, 2, 0, -2,  1, 2, 2, 1, -1, -2, -2, -1 };
const int8_t RAIO_DY[21] = { 0, -1, 0, 1,  0, -1, 1,  1, -1, -2, 0, 2,  0, -2, -1, 1, 2,  2,  1, -1, -2 };
// as 8 direções (0 = norte, no sentido dos ponteiros do relógio)
const int8_t DIR_DX[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
const int8_t DIR_DY[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };

// ---------------- memória ----------------
void * reserva(size_t n) {
  void * p = nullptr;
#if defined(BOARD_HAS_PSRAM) || defined(CONFIG_SPIRAM_SUPPORT)
  if (psramFound()) p = ps_malloc(n);
#endif
  if (!p) p = malloc(n);
  if (p) memset(p, 0, n);
  return p;
}

bool reservaMemoria() {
  mapaTer  = (uint8_t *) reserva(MAPA_N_MAX);
  mapaMelh = (uint8_t *) reserva(MAPA_N_MAX);
  mapaVis  = (uint8_t *) reserva(MAPA_N_MAX);
  mapaCont = (uint8_t *) reserva(MAPA_N_MAX);
  // memória de trabalho do gerador do mundo (fora da RAM interna, se houver PSRAM)
  extern uint8_t * genA; extern uint8_t * genB; extern uint16_t * genFila;
  genA = (uint8_t *) reserva(MAPA_N_MAX);
  genB = (uint8_t *) reserva(MAPA_N_MAX);
  genFila = (uint16_t *) reserva(MAPA_N_MAX * 2);
  if (!genA || !genB || !genFila) return false;
  U   = (Unidade *) reserva(sizeof(Unidade) * MAX_UNIDADES);
  CID = (Cidade *)  reserva(sizeof(Cidade) * MAX_CIDADES);
  return mapaTer && mapaMelh && mapaVis && mapaCont && U && CID;
}
