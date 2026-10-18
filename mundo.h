// =====================================================================
// mundo.h — IMPÉRIOS: o mapa (80x50, dá a volta de este para oeste)
// geração aleatória, rendimentos de cada casa, movimento e visibilidade
// =====================================================================
#pragma once

inline int embrulhaX(int x) { x %= MAPA_W; return x < 0 ? x + MAPA_W : x; }
inline bool dentroY(int y) { return y >= 0 && y < MAPA_H; }
inline int idxMapa(int x, int y) { return y * MAPA_W + embrulhaX(x); }

inline uint8_t terreno(int x, int y) { return dentroY(y) ? (mapaTer[idxMapa(x, y)] & 15) : T_ARTICO; }
inline bool temEspecial(int x, int y) { return dentroY(y) && (mapaTer[idxMapa(x, y)] & B_ESPECIAL); }
inline bool temAldeia(int x, int y)   { return dentroY(y) && (mapaTer[idxMapa(x, y)] & B_ALDEIA); }
inline uint8_t melh(int x, int y)     { return dentroY(y) ? mapaMelh[idxMapa(x, y)] : 0; }
inline bool eOceano(int x, int y)     { return terreno(x, y) == T_OCEANO; }
inline bool viu(int c, int x, int y)  { return dentroY(y) && ((mapaVis[idxMapa(x, y)] >> c) & 1); }
inline uint8_t continente(int x, int y) { return dentroY(y) ? mapaCont[idxMapa(x, y)] : 0; }

// distância (em casas, com a volta do mundo): a maior das duas diferenças
int distancia(int x1, int y1, int x2, int y2) {
  int dx = abs(embrulhaX(x1) - embrulhaX(x2));
  dx = min(dx, MAPA_W - dx);
  return max(dx, abs(y1 - y2));
}
// diferença em x, já com a volta do mundo (de a para b, -40..40)
int difX(int a, int b) { int d = embrulhaX(b) - embrulhaX(a); if (d > MAPA_W / 2) d -= MAPA_W; if (d < -MAPA_W / 2) d += MAPA_W; return d; }

bool juntoAoMar(int x, int y) {
  for (int d = 0; d < 8; ++d) if (dentroY(y + DIR_DY[d]) && eOceano(x + DIR_DX[d], y + DIR_DY[d])) return true;
  return false;
}

// ---------------- gerador simples (para a geração do mapa ser repetível) ----------------
uint32_t rngEstado = 1;
uint32_t aleat() { rngEstado ^= rngEstado << 13; rngEstado ^= rngEstado >> 17; rngEstado ^= rngEstado << 5; return rngEstado; }
int aleat(int n) { return n <= 0 ? 0 : (int)(aleat() % (uint32_t)n); }

// ruído suave (valores 0..255) numa grelha grossa, interpolado; dá a volta em x
void ruido(uint8_t * out, int passo, uint32_t semente) {
  rngEstado = semente ? semente : 1;
  int gw = MAPA_W / passo + 1, gh = MAPA_H / passo + 2;
  static uint8_t g[ (80 / 2 + 1) * (50 / 2 + 2) ];
  for (int i = 0; i < gw * gh; ++i) g[i] = aleat(256);
  for (int y = 0; y < MAPA_H; ++y)
    for (int x = 0; x < MAPA_W; ++x) {
      int gx = x / passo, gy = y / passo;
      int fx = (x % passo) * 256 / passo, fy = (y % passo) * 256 / passo;
      int gx2 = (gx + 1) % (MAPA_W / passo);
      int a = g[gy * gw + gx], b = g[gy * gw + gx2], c = g[(gy + 1) * gw + gx], d = g[(gy + 1) * gw + gx2];
      int top = a + ((b - a) * fx >> 8), bot = c + ((d - c) * fx >> 8);
      out[y * MAPA_W + x] = top + ((bot - top) * fy >> 8);
    }
}

// marca os continentes (enchimento) e devolve quantos há
int marcaContinentes() {
  memset(mapaCont, 0, MAPA_N);
  static uint16_t fila[MAPA_N];
  int n = 0;
  for (int i = 0; i < MAPA_N; ++i) {
    if (mapaCont[i] || (mapaTer[i] & 15) == T_OCEANO) continue;
    ++n; if (n > 250) n = 250;
    int ini = 0, fim = 0;
    fila[fim++] = i; mapaCont[i] = n;
    while (ini < fim) {
      int p = fila[ini++], x = p % MAPA_W, y = p / MAPA_W;
      for (int d = 0; d < 8; ++d) {
        int ny = y + DIR_DY[d];
        if (!dentroY(ny)) continue;
        int q = idxMapa(x + DIR_DX[d], ny);
        if (!mapaCont[q] && (mapaTer[q] & 15) != T_OCEANO) { mapaCont[q] = n; fila[fim++] = q; }
      }
    }
  }
  return n;
}

int tamanhoContinente(int cont) {
  int n = 0;
  for (int i = 0; i < MAPA_N; ++i) if (mapaCont[i] == cont) ++n;
  return n;
}

void geraMundo(uint32_t semente) {
  sementeMapa = semente;
  memset(mapaTer, T_OCEANO, MAPA_N);
  memset(mapaMelh, 0, MAPA_N);
  memset(mapaVis, 0, MAPA_N);
  static uint8_t humidade[MAPA_N], altura[MAPA_N];
  ruido(humidade, 8, semente * 7 + 3);
  ruido(altura, 5, semente * 13 + 5);
  rngEstado = semente | 1;
  for (int i = 0; i < 10; ++i) aleat();

  // 1) terra: continentes por "passeios aleatórios" até cerca de 32% do mapa
  int alvo = MAPA_N * 32 / 100, terra = 0;
  int nCont = 6 + aleat(4);
  for (int k = 0; terra < alvo && k < 400; ++k) {
    int cx, cy;
    if (k < nCont || aleat(3) == 0) { cx = aleat(MAPA_W); cy = 6 + aleat(MAPA_H - 12); }
    else {   // continuar um continente já começado
      int tent = 0;
      do { cx = aleat(MAPA_W); cy = 5 + aleat(MAPA_H - 10); } while (eOceano(cx, cy) && ++tent < 200);
    }
    int passos = 40 + aleat(80);
    for (int p = 0; p < passos; ++p) {
      for (int d = -1; d < 4; ++d) {
        int x = cx + (d == 0 ? 1 : d == 1 ? -1 : 0), y = cy + (d == 2 ? 1 : d == 3 ? -1 : 0);
        if (y < 3 || y > MAPA_H - 4) continue;
        int i = idxMapa(x, y);
        if ((mapaTer[i] & 15) == T_OCEANO) { mapaTer[i] = T_PRADO; ++terra; }
      }
      int d = aleat(8);
      cx = embrulhaX(cx + DIR_DX[d]);
      cy = constrain(cy + DIR_DY[d], 3, MAPA_H - 4);
    }
  }
  // 2) limpeza: tira ilhas de 1 casa e lagos de 1 casa
  for (int y = 1; y < MAPA_H - 1; ++y)
    for (int x = 0; x < MAPA_W; ++x) {
      int viz = 0;
      for (int d = 0; d < 8; ++d) if (!eOceano(x + DIR_DX[d], y + DIR_DY[d])) ++viz;
      int i = idxMapa(x, y);
      if (!eOceano(x, y) && viz == 0) mapaTer[i] = T_OCEANO;
      else if (eOceano(x, y) && viz == 8) mapaTer[i] = T_PRADO;
    }
  // 3) calotas polares
  for (int x = 0; x < MAPA_W; ++x) {
    mapaTer[idxMapa(x, 0)] = T_ARTICO;
    mapaTer[idxMapa(x, MAPA_H - 1)] = T_ARTICO;
    if (aleat(3) == 0) mapaTer[idxMapa(x, 1)] = T_ARTICO;
    if (aleat(3) == 0) mapaTer[idxMapa(x, MAPA_H - 2)] = T_ARTICO;
  }
  // 4) clima: latitude + humidade; relevo pela altura
  for (int y = 1; y < MAPA_H - 1; ++y)
    for (int x = 0; x < MAPA_W; ++x) {
      int i = idxMapa(x, y);
      uint8_t t = mapaTer[i] & 15;
      if (t == T_OCEANO || t == T_ARTICO) continue;
      int lat = abs(y * 2 - (MAPA_H - 1)) * 100 / (MAPA_H - 1);   // 0 equador .. 100 polo
      int h = humidade[i] + aleat(40) - 20;
      int alt = altura[i];
      uint8_t n;
      if (lat > 88) n = (aleat(2) ? T_TUNDRA : T_ARTICO);
      else if (lat > 76) n = (h > 150 ? T_FLORESTA : T_TUNDRA);
      else if (lat > 55) n = (h > 170 ? T_FLORESTA : h > 120 ? T_PRADO : h > 60 ? T_PLANICIE : T_TUNDRA);
      else if (lat > 25) n = (h > 190 ? T_FLORESTA : h > 150 ? T_PANTANO + 0 * 0 : h > 95 ? T_PRADO : h > 55 ? T_PLANICIE : T_DESERTO);
      else n = (h > 185 ? T_SELVA : h > 160 ? T_PANTANO : h > 115 ? T_PRADO : h > 70 ? T_PLANICIE : T_DESERTO);
      if (n == T_PANTANO && lat > 25 && aleat(2)) n = T_FLORESTA;
      if (alt > 205) n = T_MONTANHAS;
      else if (alt > 180) n = T_COLINAS;
      mapaTer[i] = n;
    }
  // 5) rios: nascem nas colinas e montanhas e descem até ao mar
  for (int r = 0; r < 26; ++r) {
    int x = 0, y = 0, tent = 0;
    do { x = aleat(MAPA_W); y = 3 + aleat(MAPA_H - 6); }
    while (++tent < 300 && !(terreno(x, y) == T_COLINAS || terreno(x, y) == T_MONTANHAS));
    if (tent >= 300) continue;
    int dir = aleat(8);
    for (int passo = 0; passo < 14; ++passo) {
      // vai para o vizinho mais baixo (ou o mar), sem voltar para trás
      int melhor = -1, melhorAlt = 999;
      for (int k = -2; k <= 2; k += 2) {
        int d = (dir + k + 8) & 7;
        if (d & 1) continue;            // os rios vão pelos lados das casas
        int nx = x + DIR_DX[d], ny = y + DIR_DY[d];
        if (ny < 2 || ny > MAPA_H - 3) continue;
        int a = eOceano(nx, ny) ? -1 : altura[idxMapa(nx, ny)] + aleat(30);
        if (a < melhorAlt) { melhorAlt = a; melhor = d; }
      }
      if (melhor < 0) break;
      dir = melhor;
      x = embrulhaX(x + DIR_DX[dir]); y += DIR_DY[dir];
      if (eOceano(x, y)) break;
      uint8_t t = terreno(x, y);
      if (t == T_MONTANHAS || t == T_ARTICO) continue;
      mapaTer[idxMapa(x, y)] = T_RIO;
    }
  }
  // 6) recursos especiais: espalhados num padrão regular (como no original) com um pouco de sorte
  for (int y = 1; y < MAPA_H - 1; ++y)
    for (int x = 0; x < MAPA_W; ++x) {
      int i = idxMapa(x, y);
      uint8_t t = mapaTer[i] & 15;
      bool padrao = ((x + y * 3 + (semente & 7)) % 13) == 0 || ((x * 5 + y * 7 + semente) % 37) == 0;
      if (!padrao) continue;
      if (t == T_OCEANO && !juntoAoMar(x, y)) {
        // peixe só perto de terra (onde serve a alguma cidade)
        bool perto = false;
        for (int k = 1; k < 21 && !perto; ++k) if (!eOceano(x + RAIO_DX[k], y + RAIO_DY[k])) perto = true;
        if (!perto) continue;
      }
      mapaTer[i] |= B_ESPECIAL;
    }
  // 7) aldeias tribais
  int aldeias = 0;
  for (int k = 0; k < 4000 && aldeias < 45; ++k) {
    int x = aleat(MAPA_W), y = 2 + aleat(MAPA_H - 4);
    uint8_t t = terreno(x, y);
    if (t == T_OCEANO || t == T_ARTICO) continue;
    bool perto = false;
    for (int dy = -3; dy <= 3 && !perto; ++dy)
      for (int dx = -3; dx <= 3; ++dx) if (temAldeia(x + dx, y + dy)) { perto = true; break; }
    if (perto) continue;
    mapaTer[idxMapa(x, y)] |= B_ALDEIA; ++aldeias;
  }
  marcaContinentes();
}

// ---------------- rendimentos ----------------
struct Rend { int8_t comida, escudos, comercio; };

// o que dá a casa (x, y) a uma cidade do povo civ (centro = a casa da própria cidade)
Rend rendimentoCasa(int civ, int x, int y, bool centro) {
  Rend r = { 0, 0, 0 };
  if (!dentroY(y)) return r;
  uint8_t t = terreno(x, y), m = melh(x, y);
  const InfoTerreno & I = TERRENO[t];
  bool esp = temEspecial(x, y);
  r.comida   = esp ? I.eComida   : I.comida;
  r.escudos  = esp ? I.eEscudos  : I.escudos;
  r.comercio = esp ? I.eComercio : I.comercio;
  if (t == T_PRADO && esp) { r.comida = 2; r.escudos = 1; }
  bool rega = (m & M_REGA) || (centro && I.rega);
  bool estrada = (m & M_ESTRADA) || centro;
  if (rega) r.comida += I.rega;
  if (m & M_MINA) r.escudos += I.mina;
  if (estrada) r.comercio += I.estrada;
  if (m & M_FERROVIA) r.escudos += r.escudos / 2;
  if (centro && r.escudos == 0) r.escudos = 1;
  if (m & M_POLUICAO) { r.comida /= 2; r.escudos /= 2; r.comercio /= 2; }
  int g = CIV[civ].governo;
  if (g == G_DESPOTISMO || g == G_ANARQUIA) {      // o despotismo tira 1 a quem dá mais de 2
    if (r.comida > 2) --r.comida;
    if (r.escudos > 2) --r.escudos;
    if (r.comercio > 2) --r.comercio;
  }
  if ((g == G_REPUBLICA || g == G_DEMOCRACIA) && r.comercio > 0) ++r.comercio;
  return r;
}

// ---------------- movimento ----------------
// custo (em terços) de entrar em (x2, y2) vindo de (x1, y1), por terra
bool haCidadeEm(int x, int y);
int custoMovimento(int x1, int y1, int x2, int y2, bool todoTerreno) {
  uint8_t m1 = melh(x1, y1), m2 = melh(x2, y2);
  bool c1 = haCidadeEm(x1, y1), c2 = haCidadeEm(x2, y2);
  bool f1 = (m1 & M_FERROVIA) || c1, f2 = (m2 & M_FERROVIA) || c2;
  bool e1 = (m1 & (M_ESTRADA | M_FERROVIA)) || c1, e2 = (m2 & (M_ESTRADA | M_FERROVIA)) || c2;
  if (f1 && f2 && (m1 & M_FERROVIA || m2 & M_FERROVIA)) return 0;
  if ((e1 && e2) || todoTerreno) return 1;
  return TERRENO[terreno(x2, y2)].mov * 3;
}

// ---------------- visibilidade ----------------
void revela(int c, int x, int y, int raio) {
  for (int dy = -raio; dy <= raio; ++dy) {
    int yy = y + dy;
    if (!dentroY(yy)) continue;
    for (int dx = -raio; dx <= raio; ++dx) mapaVis[idxMapa(x + dx, yy)] |= (1 << c);
  }
}
