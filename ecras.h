// =====================================================================
// ecras.h — IMPÉRIOS: desenho do mapa, painel, caixas e listas
// Ecrã de 640x200: mapa 15x13 casas (480x182) à esquerda; painel de 160
// píxeis à direita (minimapa 160x50 e informação); barra em baixo.
// =====================================================================
#pragma once

// ---------------- bitmaps (os dados ficam na memória de programa) ----------------
Bitmap BMP_TER[N_ARTE_TERRENO], BMP_ESP[N_ARTE_ESPECIAL], BMP_ICO[N_ARTE_ICONE], BMP_CID[N_ARTE_CIDADE], BMP_UNI[N_ARTE_UNIDADE];
enum { ICO_REGA, ICO_MINA, ICO_POLUICAO, ICO_FORTALEZA, ICO_ALDEIA, ICO_CURSOR };

void preparaArte() {
  for (int i = 0; i < N_ARTE_TERRENO; ++i) BMP_TER[i] = Bitmap(TILE_W, TILE_H, arte_terreno[i], PixelFormat::RGBA2222);
  for (int i = 0; i < N_ARTE_ESPECIAL; ++i) BMP_ESP[i] = Bitmap(TILE_W, TILE_H, arte_especial[i], PixelFormat::RGBA2222);
  for (int i = 0; i < N_ARTE_ICONE; ++i) BMP_ICO[i] = Bitmap(TILE_W, TILE_H, arte_icone[i], PixelFormat::RGBA2222);
  for (int i = 0; i < N_ARTE_CIDADE; ++i) BMP_CID[i] = Bitmap(TILE_W, TILE_H, arte_cidade[i], PixelFormat::RGBA2222);
  for (int i = 0; i < N_ARTE_UNIDADE; ++i) BMP_UNI[i] = Bitmap(TILE_W, TILE_H, arte_unidade[i], PixelFormat::RGBA2222);
}

// ---------------- disposição ----------------
const int ECRA_W = 640, ECRA_H = 200;
const int VIEW_W = 10, VIEW_H = 9;          // casas de 48x20: 480x180 píxeis
const int MAPA_PX_W = VIEW_W * TILE_W, MAPA_PX_H = VIEW_H * TILE_H;   // 480 x 180
const int PX = MAPA_PX_W;                                              // painel
const int BARRA_Y = 184;

int camX = 0, camY = 0;             // casa do canto de cima/esquerda da vista
int ativa = -1;                     // unidade ativa do jogador
int cursorX = 0, cursorY = 0;       // cursor do modo "ver"
bool modoVer = false;
bool piscaApagado = false;

// ---------------- o que o jogador vê agora (unidades estrangeiras) ----------------
// uma unidade estrangeira só se vê ao lado das nossas unidades ou a 2 casas das nossas cidades
uint8_t vistoAgora[(VIEW_W + 4) * (VIEW_H + 4)];
void calculaVistoAgora() {
  memset(vistoAgora, 0, sizeof(vistoAgora));
  int W = VIEW_W + 4;
  auto marca = [&](int x, int y, int r) {
    int dx0 = difX(camX, x), dy0 = y - camY;
    for (int dy = -r; dy <= r; ++dy)
      for (int dx = -r; dx <= r; ++dx) {
        int vx = dx0 + dx + 2, vy = dy0 + dy + 2;
        if (vx >= 0 && vx < W && vy >= 0 && vy < VIEW_H + 4) vistoAgora[vy * W + vx] = 1;
      }
  };
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].civ == jogador) marca(U[i].x, U[i].y, 1);
  for (int i = 0; i < MAX_CIDADES; ++i) if (CID[i].viva && CID[i].civ == jogador) marca(CID[i].x, CID[i].y, 2);
}
bool vistoAgoraEm(int x, int y) {
  int vx = difX(camX, x) + 2, vy = y - camY + 2;
  if (vx < 0 || vx >= VIEW_W + 4 || vy < 0 || vy >= VIEW_H + 4) return false;
  return vistoAgora[vy * (VIEW_W + 4) + vx];
}

// a unidade que se mostra numa casa (a ativa, ou a que melhor defende)
int unidadeMostrada(int x, int y) {
  if (ativa >= 0 && U[ativa].viva && U[ativa].x == embrulhaX(x) && U[ativa].y == y) return ativa;
  int lista[16], n = unidadesEm(x, y, lista, 16);
  if (n == 0) return -1;
  int civ = U[lista[0]].civ;
  if (civ != jogador && !vistoAgoraEm(x, y)) return -1;
  int melhor = lista[0], mv = -1;
  for (int k = 0; k < n; ++k) {
    const Unidade & u = U[lista[k]];
    int v = UNIDADE[u.tipo].defesa * 4 + (UNIDADE[u.tipo].dominio == D_MAR ? 20 : 0);
    if (v > mv) { mv = v; melhor = lista[k]; }
  }
  return melhor;
}

// ---------------- desenho de uma unidade (escudo na cor do povo + figura) ----------------
void desenhaUnidade(int sx, int sy, int i, int nNaCasa) {
  const Unidade & u = U[i];
  int cor = corDe(u.civ);
  const int x1 = sx + TILE_W - 1, y1 = sy + TILE_H - 1;
  if (nNaCasa > 1) {           // pilha: um segundo escudo por trás, deslocado
    canvas.setBrushColor(corCivE(cor));
    canvas.fillRectangle(sx + 3, sy + 2, x1, y1);
    canvas.setPenColor(C_PRETO);
    canvas.drawRectangle(sx + 3, sy + 2, x1, y1);
  }
  canvas.setBrushColor(corCiv(cor));
  canvas.fillRectangle(sx + 1, sy + 1, x1 - 2, y1 - 1);
  canvas.setPenColor(corCivE(cor));
  canvas.drawRectangle(sx + 1, sy + 1, x1 - 2, y1 - 1);
  canvas.drawBitmap(sx, sy, &BMP_UNI[u.tipo]);
  // letra do estado num quadrado preto no canto de cima, à esquerda
  char l = 0;
  switch (u.estado) {
    case EU_FORTIFICADA: case EU_FORTIFICANDO: l = 'F'; break;
    case EU_SENTINELA: l = 'S'; break;
    case EU_DESTINO: l = 'G'; break;
    case EU_TRABALHO:
      switch (u.trabalho) {
        case TB_ESTRADA: case TB_FERROVIA: l = 'R'; break;
        case TB_REGA: l = 'I'; break;
        case TB_MINA: l = 'M'; break;
        case TB_FORTALEZA: l = 'F'; break;
        case TB_LIMPAR: l = 'P'; break;
      }
      break;
  }
  if (l) {
    char s[2] = { l, 0 };
    canvas.setBrushColor(C_PRETO);
    canvas.fillRectangle(sx + 2, sy + 2, sx + 10, sy + 10);
    canvas.setPenColor(C_BRANCO);
    canvas.drawText(&fonte, sx + 3, sy + 2, s);
  }
  if (u.vet) {                                  // veterana: estrela dourada no canto
    canvas.setPenColor(C_OURO);
    canvas.drawLine(x1 - 6, sy + 3, x1 - 4, sy + 3);
    canvas.drawLine(x1 - 5, sy + 2, x1 - 5, sy + 4);
  }
  if (u.estado == EU_FORTIFICADA) {            // moldura de fortificação
    canvas.setPenColor(C_CINZA);
    canvas.drawRectangle(sx, sy, x1, y1);
    canvas.setPenColor(C_CINZA_E);
    canvas.drawRectangle(sx + 1, sy + 1, x1 - 1, y1 - 1);
  }
}

// ---------------- desenho de uma casa ----------------
bool temEstradaOuCidade(int x, int y) { return dentroY(y) && ((melh(x, y) & (M_ESTRADA | M_FERROVIA)) || haCidadeEm(x, y)); }

// linha grossa do centro da casa até ao meio de um lado ou canto (rios e estradas)
void linhaGrossa(int cx, int cy, int ex, int ey, RGB888 cor, RGB888 brilho, bool comBrilho) {
  canvas.setPenColor(cor);
  canvas.drawLine(cx, cy, ex, ey);
  canvas.drawLine(cx + 1, cy, ex + 1, ey);
  canvas.drawLine(cx, cy + 1, ex, ey + 1);
  canvas.drawLine(cx + 1, cy + 1, ex + 1, ey + 1);
  if (comBrilho) { canvas.setPenColor(brilho); canvas.drawLine(cx, cy, ex, ey); }
}

void desenhaCasa(int sx, int sy, int x, int y, bool comUnidade) {
  x = embrulhaX(x);
  const int x1 = sx + TILE_W - 1, y1 = sy + TILE_H - 1;
  if (!dentroY(y) || !viu(jogador, x, y)) {
    canvas.setBrushColor(C_PRETO);
    canvas.fillRectangle(sx, sy, x1, y1);
    return;
  }
  uint8_t t = terreno(x, y);
  canvas.drawBitmap(sx, sy, &BMP_TER[t]);
  const int cx = sx + TILE_W / 2 - 1, cy = sy + TILE_H / 2 - 1;
  const int mx = TILE_W / 2, my = TILE_H / 2;          // do centro até ao lado
  if (t == T_OCEANO) {
    // costa: faixa de água clara do lado da terra, com espuma branca
    canvas.setBrushColor(C_COSTA);
    bool n = dentroY(y - 1) && !eOceano(x, y - 1), s = dentroY(y + 1) && !eOceano(x, y + 1);
    bool o = !eOceano(x - 1, y), e = !eOceano(x + 1, y);
    if (n) canvas.fillRectangle(sx, sy, x1, sy + 2);
    if (s) canvas.fillRectangle(sx, y1 - 2, x1, y1);
    if (o) canvas.fillRectangle(sx, sy, sx + 5, y1);
    if (e) canvas.fillRectangle(x1 - 5, sy, x1, y1);
    canvas.setPenColor(C_BRANCO);
    if (n) canvas.drawLine(sx, sy, x1, sy);
    if (s) canvas.drawLine(sx, y1, x1, y1);
    if (o) canvas.drawLine(sx, sy, sx, y1);
    if (e) canvas.drawLine(x1, sy, x1, y1);
  }
  if (t == T_RIO) {
    // o rio liga aos rios e ao mar ao lado (norte, sul, este, oeste)
    int liga = 0;
    const int8_t ox[4] = { 0, 1, 0, -1 }, oy[4] = { -1, 0, 1, 0 };
    for (int d = 0; d < 4; ++d) {
      int nx = x + ox[d], ny = y + oy[d];
      if (!dentroY(ny)) continue;
      uint8_t tv = terreno(nx, ny);
      if (tv != T_RIO && tv != T_OCEANO) continue;
      ++liga;
      linhaGrossa(cx, cy, cx + ox[d] * mx, cy + oy[d] * my, C_RIO, C_CEU, true);
    }
    if (liga == 0) linhaGrossa(sx + 6, cy, x1 - 6, cy, C_RIO, C_CEU, true);
    canvas.setBrushColor(C_RIO);
    canvas.fillRectangle(cx - 2, cy - 1, cx + 3, cy + 2);
  }
  uint8_t m = melh(x, y);
  if (temEspecial(x, y)) canvas.drawBitmap(sx, sy, &BMP_ESP[t]);
  if (m & M_REGA) canvas.drawBitmap(sx, sy, &BMP_ICO[ICO_REGA]);
  if (m & M_MINA) canvas.drawBitmap(sx, sy, &BMP_ICO[ICO_MINA]);
  int ci = cidadeEm(x, y);
  if ((m & (M_ESTRADA | M_FERROVIA)) && ci < 0) {
    bool algum = false;
    for (int d = 0; d < 8; ++d) {
      int nx = x + DIR_DX[d], ny = y + DIR_DY[d];
      if (!temEstradaOuCidade(nx, ny)) continue;
      algum = true;
      bool ferro = (m & M_FERROVIA) && ((melh(nx, ny) & M_FERROVIA) || haCidadeEm(nx, ny));
      int ex = cx + DIR_DX[d] * mx, ey = cy + DIR_DY[d] * my;
      if (ferro) {
        linhaGrossa(cx, cy, ex, ey, C_PRETO, C_CINZA, true);     // carris
      } else {
        linhaGrossa(cx, cy, ex, ey, C_ESTRADA, RGB888(255, 170, 85), true);
      }
    }
    if (!algum) { canvas.setBrushColor(C_ESTRADA); canvas.fillRectangle(cx - 2, cy - 1, cx + 3, cy + 2); }
  }
  if (m & M_POLUICAO) canvas.drawBitmap(sx, sy, &BMP_ICO[ICO_POLUICAO]);
  if (m & M_FORTALEZA) canvas.drawBitmap(sx, sy, &BMP_ICO[ICO_FORTALEZA]);
  if (temAldeia(x, y)) canvas.drawBitmap(sx, sy, &BMP_ICO[ICO_ALDEIA]);
  if (ci >= 0) {
    const Cidade & k = CID[ci];
    int cor = corDe(k.civ);
    canvas.setBrushColor(corCiv(cor));
    canvas.fillRectangle(sx + 1, sy + 1, x1 - 1, y1 - 1);
    bool muralha = temEdif(k, E_MURALHAS);
    canvas.drawBitmap(sx, sy, &BMP_CID[muralha ? 1 : 0]);
    canvas.setPenColor(corCivE(cor));
    if (!muralha) canvas.drawRectangle(sx, sy, x1, y1);
    // tamanho num quadrado preto, em cima à esquerda
    char b[4]; sprintf(b, "%d", k.tam);
    int w = strlen(b) * 8;
    canvas.setBrushColor(C_PRETO);
    canvas.fillRectangle(sx + 1, sy + 1, sx + w + 2, sy + 10);
    canvas.setPenColor(C_BRANCO);
    canvas.drawText(&fonte, sx + 2, sy + 2, b);
    // há unidades dentro: uma bandeira no canto de cima, à direita
    if (comUnidade && primeiraUnidadeEm(x, y) >= 0 && (k.civ == jogador || vistoAgoraEm(x, y))) {
      canvas.setPenColor(C_PRETO);
      canvas.drawLine(x1 - 7, sy + 1, x1 - 7, sy + 9);
      canvas.setBrushColor(corCivE(cor));
      canvas.fillRectangle(x1 - 6, sy + 1, x1 - 2, sy + 4);
    }
  }
  if (comUnidade && ci < 0) {
    int u = unidadeMostrada(x, y);
    if (u >= 0) {
      int lista[16], n = unidadesEm(x, y, lista, 16);
      desenhaUnidade(sx, sy, u, n);
    }
  } else if (comUnidade && ci >= 0 && ativa >= 0 && U[ativa].viva && U[ativa].x == x && U[ativa].y == y) {
    desenhaUnidade(sx, sy, ativa, 1);
  }
}

// nomes das cidades por baixo (depois de desenhar as casas)
void desenhaNomesCidades() {
  for (int i = 0; i < MAX_CIDADES; ++i) {
    const Cidade & k = CID[i];
    if (!k.viva || !viu(jogador, k.x, k.y)) continue;
    int vx = difX(camX, k.x), vy = k.y - camY;
    if (vx < 0 || vx >= VIEW_W || vy < 0 || vy >= VIEW_H) continue;
    int n = letras(k.nome);
    int x = vx * TILE_W + TILE_W / 2 - n * 4, y = vy * TILE_H + TILE_H;
    x = constrain(x, 0, MAPA_PX_W - n * 8);
    if (y + 8 > MAPA_PX_H) y = vy * TILE_H - 8;
    texto(x + 1, y + 1, k.nome, C_PRETO);
    texto(x, y, k.nome, corCiv(corDe(k.civ)));
  }
}

// linha branca à volta de todo o ecrã (desenha-se outra vez depois de cada redesenho junto às bordas)
void moldura() {
  canvas.setPenColor(C_BRANCO);
  canvas.drawRectangle(0, 0, ECRA_W - 1, ECRA_H - 1);
}

void desenhaMapa() {
  calculaVistoAgora();
  for (int vy = 0; vy < VIEW_H; ++vy)
    for (int vx = 0; vx < VIEW_W; ++vx)
      desenhaCasa(vx * TILE_W, vy * TILE_H, camX + vx, camY + vy, true);
  desenhaNomesCidades();
  if (modoVer) {
    int vx = difX(camX, cursorX), vy = cursorY - camY;
    if (vx >= 0 && vx < VIEW_W && vy >= 0 && vy < VIEW_H) canvas.drawBitmap(vx * TILE_W, vy * TILE_H, &BMP_ICO[ICO_CURSOR]);
  }
  moldura();
}

// redesenha só uma casa (e a de baixo, por causa dos nomes)
void redesenhaCasa(int x, int y) {
  int vx = difX(camX, x), vy = y - camY;
  if (vx < 0 || vx >= VIEW_W || vy < 0 || vy >= VIEW_H) return;
  desenhaCasa(vx * TILE_W, vy * TILE_H, x, y, true);
  moldura();
}

// a casa (x, y) está na vista (com margem)?
bool naVista(int x, int y, int margem) {
  int vx = difX(camX, x), vy = y - camY;
  return vx >= margem && vx < VIEW_W - margem && vy >= margem && vy < VIEW_H - margem;
}
void centraEm(int x, int y) {
  camX = embrulhaX(x - VIEW_W / 2);
  camY = constrain(y - VIEW_H / 2, 0, MAPA_H - VIEW_H);
}

// ---------------- minimapa (2x1 píxeis por casa) ----------------
RGB888 corMinimapa(uint8_t t) {
  switch (t) {
    case T_OCEANO: return C_AZUL_E;
    case T_PRADO: return RGB888(0, 170, 0);
    case T_PLANICIE: return RGB888(170, 170, 85);
    case T_DESERTO: return RGB888(255, 255, 85);
    case T_TUNDRA: return C_CINZA;
    case T_ARTICO: return C_BRANCO;
    case T_FLORESTA: return RGB888(0, 85, 0);
    case T_SELVA: return RGB888(0, 85, 0);
    case T_PANTANO: return RGB888(0, 85, 85);
    case T_COLINAS: return RGB888(85, 170, 0);
    case T_MONTANHAS: return C_CINZA_E;
    case T_RIO: return RGB888(0, 170, 0);
  }
  return C_PRETO;
}
void desenhaMinimapa() {
  for (int y = 0; y < MAPA_H; ++y) {
    int x = 0;
    while (x < MAPA_W) {
      // junta casas seguidas com a mesma cor
      bool v = viu(jogador, x, y);
      uint8_t t = terreno(x, y);
      int x2 = x + 1;
      while (x2 < MAPA_W && viu(jogador, x2, y) == v && (!v || terreno(x2, y) == t)) ++x2;
      canvas.setBrushColor(v ? corMinimapa(t) : C_PRETO);
      canvas.fillRectangle(PX + x * 2, y, PX + x2 * 2 - 1, y);
      x = x2;
    }
  }
  for (int i = 0; i < MAX_CIDADES; ++i) {
    const Cidade & k = CID[i];
    if (!k.viva || !viu(jogador, k.x, k.y)) continue;
    canvas.setBrushColor(corCiv(corDe(k.civ)));
    canvas.fillRectangle(PX + k.x * 2, k.y, PX + k.x * 2 + 1, k.y);
  }
  // retângulo da vista
  canvas.setPenColor(C_BRANCO);
  int x0 = PX + camX * 2, x1 = x0 + VIEW_W * 2 - 1, y0 = camY, y1 = camY + VIEW_H - 1;
  if (x1 < PX + 160) canvas.drawRectangle(x0, y0, x1, y1);
  else {
    canvas.drawLine(x0, y0, PX + 159, y0); canvas.drawLine(x0, y1, PX + 159, y1); canvas.drawLine(x0, y0, x0, y1);
    int xr = x1 - 160;
    canvas.drawLine(PX, y0, xr, y0); canvas.drawLine(PX, y1, xr, y1); canvas.drawLine(xr, y0, xr, y1);
  }
}

// ---------------- painel ----------------
void desenhaInfoUnidade(int y) {
  if (ativa < 0 || !U[ativa].viva) {
    texto(PX + 4, y, "Sem ordens a dar.", C_CINZA);
    texto(PX + 4, y + 18, "ENTER: fim do turno", C_AMARELO);
    return;
  }
  const Unidade & u = U[ativa];
  const InfoUnidade & I = UNIDADE[u.tipo];
  texto(PX + 4, y, I.nome, C_BRANCO);
  char b[40];
  sprintf(b, "%d/%d/%d%s", I.ataque, I.defesa, I.mov, u.vet ? "  Vet." : "");
  texto(PX + 4, y + 9, b, C_CINZA);
  int m = u.mov;
  if (m % 3) sprintf(b, "Mov. %d %d/3", m / 3, m % 3); else sprintf(b, "Mov. %d", m / 3);
  texto(PX + 4, y + 18, b, C_CINZA);
  if (u.casa != SEM && u.casa < MAX_CIDADES && CID[u.casa].viva) texto(PX + 4, y + 27, CID[u.casa].nome, C_CINZA);
  if (I.combustivel) { sprintf(b, "Combustível %d", u.comb); texto(PX + 84, y + 18, b, C_CINZA); }
}

void desenhaInfoTerreno(int y, int x, int yy) {
  if (!dentroY(yy) || !viu(jogador, x, yy)) return;
  uint8_t t = terreno(x, yy), m = melh(x, yy);
  char b[48];
  if (temEspecial(x, yy)) sprintf(b, "%s (%s)", TERRENO[t].nome, TERRENO[t].especial);
  else strcpy(b, TERRENO[t].nome);
  texto(PX + 4, y, b, C_VERDE);
  b[0] = 0;
  if (m & M_FERROVIA) strcat(b, "Ferrovia "); else if (m & M_ESTRADA) strcat(b, "Estrada ");
  if (m & M_REGA) strcat(b, "Rega ");
  if (m & M_MINA) strcat(b, "Mina ");
  if (m & M_FORTALEZA) strcat(b, "Fortaleza ");
  if (m & M_POLUICAO) strcat(b, "Poluição");
  if (b[0]) texto(PX + 4, y + 9, b, C_CINZA);
}

void desenhaPainel() {
  retangulo(PX, 0, ECRA_W - 1, ECRA_H - 1, C_PAINEL);
  desenhaMinimapa();
  canvas.setPenColor(C_MOLDURA);
  canvas.drawLine(PX, 50, ECRA_W - 1, 50);
  const Civ & C = CIV[jogador];
  char b[48], ano[24];
  textoAno(ano, turno);
  texto(PX + 4, 53, ano, C_AMARELO);
  sprintf(b, "%ld$", (long)C.ouro);
  textoDireita(ECRA_W - 4, 53, b, C_OURO);
  sprintf(b, "%s %d.%d.%d", GOVERNO[C.governo].nome, C.taxa, C.luxo, 10 - C.taxa - C.luxo);
  texto(PX + 4, 62, b, C_CINZA);
  // ciência: barra
  int custo = custoTec(jogador);
  texto(PX + 4, 71, C.pesquisa == SEM_TEC ? "(sem pesquisa)" : TEC[C.pesquisa].nome, C_CEU);
  canvas.setPenColor(C_CINZA_E);
  canvas.drawRectangle(PX + 4, 80, ECRA_W - 5, 83);
  int w = custo > 0 ? min(150, C.lampadas * 150 / custo) : 0;
  if (w > 0) retangulo(PX + 5, 81, PX + 4 + w, 82, C_CEU);
  canvas.setPenColor(C_MOLDURA);
  canvas.drawLine(PX, 86, ECRA_W - 1, 86);
  desenhaInfoUnidade(89);
  canvas.setPenColor(C_MOLDURA);
  canvas.drawLine(PX, 127, ECRA_W - 1, 127);
  int tx = modoVer ? cursorX : (ativa >= 0 && U[ativa].viva ? U[ativa].x : -1);
  int ty = modoVer ? cursorY : (ativa >= 0 && U[ativa].viva ? U[ativa].y : -1);
  if (tx >= 0) {
    desenhaInfoTerreno(130, tx, ty);
    // outras unidades na casa
    int lista[16], n = unidadesEm(tx, ty, lista, 16), y = 150;
    int ci = cidadeEm(tx, ty);
    if (ci >= 0 && (CID[ci].civ == jogador || viu(jogador, tx, ty))) { texto(PX + 4, y, CID[ci].nome, corCiv(corDe(CID[ci].civ))); y += 9; }
    if (n > 1 && (U[lista[0]].civ == jogador)) {
      sprintf(b, "%d unidades aqui", n);
      texto(PX + 4, y, b, C_CINZA);
    }
  }
  moldura();
}

// ---------------- barra de baixo ----------------
char barra1[90] = "", barra2[90] = "";
void desenhaBarra() {
  retangulo(0, MAPA_PX_H, MAPA_PX_W - 1, ECRA_H - 1, C_PRETO);
  texto(2, BARRA_Y, barra1, C_BRANCO);
  texto(2, BARRA_Y + 8, barra2, C_CINZA);
  moldura();
}
void poeBarra(const char * a, const char * b) {
  strncpy(barra1, a, sizeof(barra1) - 1); barra1[sizeof(barra1) - 1] = 0;
  strncpy(barra2, b, sizeof(barra2) - 1); barra2[sizeof(barra2) - 1] = 0;
  desenhaBarra();
}

void desenhaTudo() {
  desenhaMapa();
  desenhaPainel();
  desenhaBarra();
}

// ---------------- caixas ----------------
// caixa de mensagem a meio do mapa; devolve a altura usada
void caixaTexto(const char * titulo, const char * txt, const char * rodape, RGB888 corTitulo = C_AMARELO) {
  int larg = 46;
  int linhas = linhasTexto(larg, txt);
  int h = linhas * 9 + 14 + (titulo ? 11 : 0) + (rodape ? 10 : 0);
  int w = larg * 8 + 16;
  int x0 = (MAPA_PX_W - w) / 2, y0 = max(2, (MAPA_PX_H - h) / 2);
  caixa(x0, y0, x0 + w, y0 + h, C_PAINEL, C_MOLDURA);
  int y = y0 + 6;
  if (titulo) { textoCentro(x0, x0 + w, y, titulo, corTitulo); y += 11; }
  y += textoQuebrado(x0 + 8, y, larg, txt, C_BRANCO);
  if (rodape) textoDireita(x0 + w - 8, y0 + h - 11, rodape, C_CINZA);
}

// ---------------- lista de escolha (genérica) ----------------
const int MAX_ITENS = 100;
struct Item { char texto[44]; char direita[16]; int valor; bool ativo; };
Item itens[MAX_ITENS];
int nItens = 0, itemSel = 0, itemTopo = 0;
char tituloLista[64] = "";
char ajudaLista[96] = "";
int idLista = 0;                 // o que a lista está a escolher (ver jogo.h)
int listaX0 = 60, listaW = 360, listaLinhas = 15;
int listaY0 = -1;                // -1 = ao meio do ecrã

void limpaLista(const char * titulo, int id) {
  nItens = 0; itemSel = 0; itemTopo = 0; idLista = id; listaY0 = -1;
  strncpy(tituloLista, titulo, sizeof(tituloLista) - 1); tituloLista[sizeof(tituloLista) - 1] = 0;
  ajudaLista[0] = 0;
}
void juntaItem(const char * t, int valor, bool ativo = true, const char * dir = nullptr) {
  if (nItens >= MAX_ITENS) return;
  Item & it = itens[nItens++];
  strncpy(it.texto, t, sizeof(it.texto) - 1); it.texto[sizeof(it.texto) - 1] = 0;
  if (dir) { strncpy(it.direita, dir, sizeof(it.direita) - 1); it.direita[sizeof(it.direita) - 1] = 0; } else it.direita[0] = 0;
  it.valor = valor; it.ativo = ativo;
}

void desenhaLista() {
  int linhas = min(nItens, listaLinhas);
  int topo = tituloLista[0] ? 15 : 5;             // sem título, a lista começa logo
  int h = linhas * 10 + topo + 5 + (ajudaLista[0] ? 10 : 0);
  int x0 = listaX0, x1 = listaX0 + listaW;
  int y0 = listaY0 >= 0 ? listaY0 : max(1, (ECRA_H - h) / 2 - 4);
  caixa(x0, y0, x1, y0 + h, C_PAINEL, C_MOLDURA);
  textoCentro(x0, x1, y0 + 4, tituloLista, C_AMARELO);
  if (itemSel < itemTopo) itemTopo = itemSel;
  if (itemSel >= itemTopo + linhas) itemTopo = itemSel - linhas + 1;
  for (int k = 0; k < linhas; ++k) {
    int i = itemTopo + k;
    if (i >= nItens) break;
    int y = y0 + topo + k * 10;
    bool sel = i == itemSel;
    if (sel) retangulo(x0 + 4, y - 1, x1 - 4, y + 8, C_MOLDURA);
    RGB888 cor = !itens[i].ativo ? C_CINZA_E : sel ? C_AMARELO : C_BRANCO;
    texto(x0 + 8, y, itens[i].texto, cor);
    if (itens[i].direita[0]) textoDireita(x1 - 8, y, itens[i].direita, sel ? C_AMARELO : C_CINZA);
  }
  // setas de scroll
  if (itemTopo > 0) texto(x1 - 12, y0 + 4, "^", C_CINZA);
  if (itemTopo + linhas < nItens) texto(x1 - 12, y0 + h - 10, "v", C_CINZA);
  if (ajudaLista[0]) texto(x0 + 8, y0 + h - 10, ajudaLista, C_CINZA);
}

// devolve: -1 nada; -2 cancelou; >= 0 índice escolhido
int atualizaLista() {
  bool mudou = false;
  if (menuCima() && nItens) { itemSel = (itemSel + nItens - 1) % nItens; mudou = true; }
  if (menuBaixo() && nItens) { itemSel = (itemSel + 1) % nItens; mudou = true; }
  if (premiu(VK_PAGEUP)) { itemSel = max(0, itemSel - listaLinhas); mudou = true; }
  if (premiu(VK_PAGEDOWN)) { itemSel = min(nItens - 1, itemSel + listaLinhas); mudou = true; }
  if (mudou) { somMenu(); desenhaLista(); }
  if (carregouVolta()) return -2;
  if (carregouFogo() && nItens) {
    if (!itens[itemSel].ativo) { somErro(); return -1; }
    somOk();
    return itemSel;
  }
  return -1;
}
