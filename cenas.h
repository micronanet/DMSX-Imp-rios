// =====================================================================
// cenas.h — IMPÉRIOS: cenas animadas
//   - uma maravilha construída (13 cenas, uma por tipo de monumento)
//   - uma cidade inimiga conquistada (as tropas entram, a bandeira muda, o saque)
//   - uma descoberta dos sábios (o sábio da época no seu gabinete tem uma ideia)
// Tudo desenhado com rectângulos, elipses e polígonos da FabGL. Os monumentos "crescem" de baixo para
// cima redesenhando-os com um recorte (clipping) que só deixa pintar a faixa nova: nada pisca.
// ESPAÇO salta a animação; no fim, ESPAÇO continua o jogo.
// =====================================================================
#pragma once

// ---------------- utilitários ----------------
inline RGB888 cr(int r, int g, int b) { return RGB888(r, g, b); }
void recorta(int x0, int y0, int x1, int y1) { canvas.setClippingRect(Rect(max(0, x0), max(0, y0), min(ECRA_W - 1, x1), min(ECRA_H - 1, y1))); }
void semRecorte() { canvas.setClippingRect(Rect(0, 0, ECRA_W - 1, ECRA_H - 1)); }
void ret(int x0, int y0, int x1, int y1, RGB888 c) { canvas.setBrushColor(c); canvas.fillRectangle(x0, y0, x1, y1); }
void tri(int x0, int y0, int x1, int y1, int x2, int y2, RGB888 c) {
  Point p[3] = { Point(x0, y0), Point(x1, y1), Point(x2, y2) };
  canvas.setBrushColor(c); canvas.fillPath(p, 3);
}
void quad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, RGB888 c) {
  Point p[4] = { Point(x0, y0), Point(x1, y1), Point(x2, y2), Point(x3, y3) };
  canvas.setBrushColor(c); canvas.fillPath(p, 4);
}
void elip(int cx, int cy, int w, int h, RGB888 c) { canvas.setBrushColor(c); canvas.fillEllipse(cx, cy, w, h); }
void lin(int x0, int y0, int x1, int y1, RGB888 c) { canvas.setPenColor(c); canvas.drawLine(x0, y0, x1, y1); }

// céu em faixas, com uma linha de meio-tom entre faixas
void ceuFaixas(int y0, int y1, const RGB888 * c, int n) {
  for (int k = 0; k < n; ++k) {
    int a = y0 + (y1 - y0 + 1) * k / n, b = y0 + (y1 - y0 + 1) * (k + 1) / n - 1;
    ret(0, a, ECRA_W - 1, b, c[k]);
    if (k > 0) for (int x = (a & 1); x < ECRA_W; x += 2) canvas.setPixel(x, a, c[k - 1]);
  }
}
const RGB888 CEU_DIA[4]    = { RGB888(0, 0, 170), RGB888(0, 85, 255), RGB888(85, 170, 255), RGB888(170, 255, 255) };
const RGB888 CEU_POENTE[5] = { RGB888(85, 0, 85), RGB888(170, 0, 85), RGB888(255, 85, 0), RGB888(255, 170, 0), RGB888(255, 255, 85) };
const RGB888 CEU_NOITE[3]  = { RGB888(0, 0, 0), RGB888(0, 0, 85), RGB888(0, 0, 170) };
const RGB888 CEU_MADRUGADA[4] = { RGB888(0, 0, 85), RGB888(85, 0, 170), RGB888(170, 85, 170), RGB888(255, 170, 85) };

const int CN_HORIZ = 118, CN_BASE = 146, CN_FIM_Y = 155;     // horizonte, chão dos monumentos, fim da área da cena

void estrelas(int n, uint32_t sem) {
  uint32_t r = sem;
  for (int i = 0; i < n; ++i) {
    r = r * 1103515245 + 12345; int x = (r >> 8) % ECRA_W;
    r = r * 1103515245 + 12345; int y = (r >> 8) % (CN_HORIZ - 10);
    canvas.setPixel(x, y, (i % 5) ? cr(255, 255, 255) : cr(255, 255, 85));
  }
}
void chao(RGB888 claro, RGB888 escuro, int y0 = CN_HORIZ) {
  ret(0, y0, ECRA_W - 1, CN_FIM_Y, claro);
  for (int y = y0 + 3; y <= CN_FIM_Y; y += 4)
    for (int x = ((y / 4) & 1) * 6; x < ECRA_W; x += 12) ret(x, y, x + 2, y, escuro);
}
void mar(int y0 = CN_HORIZ) {
  ret(0, y0, ECRA_W - 1, CN_FIM_Y, cr(0, 0, 170));
  for (int y = y0 + 2; y <= CN_FIM_Y; y += 3)
    for (int x = (y * 7) % 23; x < ECRA_W; x += 23) ret(x, y, x + 6, y, cr(85, 170, 255));
}
void sol(int cx, int cy, int r, RGB888 c, RGB888 brilho) { elip(cx, cy, r * 5, r * 2, brilho); elip(cx, cy, r * 4, r * 2 - 2, c); }

// ---------------- os monumentos (desenho completo; a revelação faz-se com recortes) ----------------
enum TipoMonumento : uint8_t { MO_PIRAMIDE, MO_ZIGURATE, MO_ESTATUA, MO_FAROL, MO_MURALHA, MO_TEMPLO, MO_CATEDRAL,
                               MO_OBSERVATORIO, MO_NAVIO, MO_BARRAGEM, MO_FOGUETAO, MO_COGUMELO, MO_ARRANHA };
// tipo de cena de cada maravilha (pela ordem de MARAVILHA em dados.h)
const uint8_t MONUMENTO[N_MARAVILHAS] = {
  MO_PIRAMIDE, MO_ESTATUA, MO_TEMPLO, MO_MURALHA, MO_ZIGURATE, MO_FAROL, MO_TEMPLO, MO_OBSERVATORIO,
  MO_CATEDRAL, MO_NAVIO, MO_CATEDRAL, MO_NAVIO, MO_TEMPLO, MO_CATEDRAL, MO_OBSERVATORIO, MO_TEMPLO,
  MO_NAVIO, MO_TEMPLO, MO_BARRAGEM, MO_COGUMELO, MO_ARRANHA, MO_FOGUETAO, MO_ARRANHA };

void pedraLinhas(int x0, int x1, int y0, int y1, int passo, RGB888 c) {
  for (int y = y1 - passo; y > y0; y -= passo) lin(x0, y, x1, y, c);
}

void piramide(int cx, int base, int meia, int alt) {
  const RGB888 luz = cr(255, 255, 170), sombra = cr(255, 170, 0), junta = cr(170, 85, 0);
  tri(cx - meia, base, cx, base - alt, cx, base, luz);
  tri(cx, base, cx, base - alt, cx + meia, base, sombra);
  for (int y = base - 4; y > base - alt; y -= 4) {
    int hw = meia * (y - (base - alt)) / alt;
    lin(cx - hw, y, cx + hw, y, junta);
    for (int x = cx - hw + ((y / 4) & 1) * 6; x < cx; x += 12) lin(x, y, x, y + 3, junta);
  }
  lin(cx, base, cx, base - alt, junta);
}
void desenhaMonumento(int tipo, int w) {
  const int cx = 320, b = CN_BASE;
  switch (tipo) {
    case MO_PIRAMIDE:
      piramide(cx - 190, b - 8, 70, 38);
      piramide(cx + 200, b - 6, 85, 46);
      piramide(cx, b, 160, 92);
      break;
    case MO_ZIGURATE: {      // jardins em socalcos, com plantas a cair das bordas
      const RGB888 tijolo = cr(170, 85, 0), luz = cr(255, 170, 85), verde = cr(0, 170, 0), verdeC = cr(85, 255, 85);
      for (int k = 0; k < 4; ++k) {
        int hw = 190 - k * 42, y1 = b - k * 22, y0 = y1 - 22;
        ret(cx - hw, y0, cx + hw, y1, tijolo);
        ret(cx - hw, y0, cx - hw + 6, y1, luz);
        pedraLinhas(cx - hw, cx + hw, y0, y1, 5, cr(85, 0, 0));
        for (int x = cx - hw + 4; x < cx + hw; x += 11) { ret(x, y0, x + 6, y0 + 3, verde); ret(x + 2, y0 + 3, x + 4, y0 + 7 + (x % 3) * 2, verde); ret(x + 1, y0, x + 3, y0 + 1, verdeC); }
      }
      ret(cx - 14, b - 88, cx + 14, b, cr(255, 170, 85));                     // escadaria
      pedraLinhas(cx - 14, cx + 14, b - 88, b, 3, tijolo);
      break;
    }
    case MO_ESTATUA: {       // o Colosso, de pernas abertas sobre a entrada do porto, com o archote
      const RGB888 br = cr(170, 85, 0), brL = cr(255, 170, 0), brE = cr(85, 0, 0), ped = cr(170, 170, 170);
      ret(cx - 120, b - 20, cx - 60, b, ped); ret(cx + 60, b - 20, cx + 120, b, ped);
      quad(cx - 96, b - 20, cx - 80, b - 20, cx - 6, b - 70, cx - 22, b - 70, br);     // pernas
      quad(cx + 96, b - 20, cx + 80, b - 20, cx + 6, b - 70, cx + 22, b - 70, brE);
      quad(cx - 30, b - 70, cx + 30, b - 70, cx + 24, b - 110, cx - 24, b - 110, br);  // tronco
      ret(cx + 4, b - 108, cx + 24, b - 72, brE);
      ret(cx - 26, b - 80, cx + 26, b - 76, brL);                                       // cinto
      elip(cx, b - 118, 26, 14, brL);                                                   // cabeça
      for (int k = -3; k <= 3; ++k) lin(cx + k * 4, b - 124, cx + k * 9, b - 132, cr(255, 255, 85));   // coroa de raios
      quad(cx + 22, b - 108, cx + 30, b - 110, cx + 52, b - 136, cx + 44, b - 136, br); // braço erguido
      elip(cx + 49, b - 142, 14, 10, cr(255, 85, 0)); elip(cx + 49, b - 144, 6, 5, cr(255, 255, 85));
      quad(cx - 24, b - 106, cx - 30, b - 104, cx - 44, b - 80, cx - 38, b - 78, brE);  // o outro braço
      break;
    }
    case MO_FAROL: {
      const RGB888 pedra = cr(255, 255, 255), sombra = cr(170, 170, 170), escura = cr(85, 85, 85);
      elip(cx, b + 4, 220, 26, cr(85, 85, 85)); elip(cx - 20, b, 160, 16, cr(170, 170, 170));   // rochedo
      ret(cx - 34, b - 50, cx + 34, b - 4, pedra); ret(cx + 14, b - 50, cx + 34, b - 4, sombra);
      pedraLinhas(cx - 34, cx + 34, b - 50, b - 4, 6, sombra);
      ret(cx - 24, b - 86, cx + 24, b - 50, pedra); ret(cx + 10, b - 86, cx + 24, b - 50, sombra);
      ret(cx - 14, b - 110, cx + 14, b - 86, pedra); ret(cx + 6, b - 110, cx + 14, b - 86, sombra);
      for (int y = b - 44; y > b - 104; y -= 14) ret(cx - 3, y, cx + 3, y + 5, escura);
      ret(cx - 18, b - 114, cx + 18, b - 110, escura);
      elip(cx, b - 122, 30, 14, cr(255, 85, 0)); elip(cx, b - 124, 16, 8, cr(255, 255, 85));
      break;
    }
    case MO_MURALHA: {       // a muralha a serpentear pelas colinas, com torres
      for (int k = 0; k < 6; ++k) elip(60 + k * 110, CN_HORIZ + 8, 200, 40, k & 1 ? cr(0, 170, 0) : cr(0, 85, 0));
      for (int x = 0; x < ECRA_W; x += 2) {
        int y = CN_HORIZ - 4 + (int)(10 * sin(x * 0.013) + 5 * sin(x * 0.041));
        ret(x, y, x + 1, y + 14, (x / 2) % 2 ? cr(170, 170, 170) : cr(255, 255, 255));
        ret(x, y + 14, x + 1, y + 16, cr(85, 85, 85));
        if ((x / 6) % 2 == 0) ret(x, y - 3, x + 1, y - 1, cr(170, 170, 170));
        if (x % 120 == 40) { ret(x - 6, y - 14, x + 8, y + 14, cr(170, 170, 170)); ret(x - 6, y - 14, x - 2, y + 14, cr(255, 255, 255)); ret(x - 8, y - 17, x + 10, y - 14, cr(85, 85, 85)); }
      }
      break;
    }
    case MO_TEMPLO: {        // templo clássico de mármore
      const RGB888 marm = cr(255, 255, 255), som = cr(170, 170, 170), esc = cr(85, 85, 85);
      for (int k = 0; k < 3; ++k) { ret(cx - 170 + k * 8, b - 6 * (k + 1), cx + 170 - k * 8, b - 6 * k, k & 1 ? som : marm); }
      ret(cx - 150, b - 82, cx + 150, b - 18, esc);                              // interior na sombra
      for (int k = 0; k < 8; ++k) {
        int x = cx - 146 + k * 40;
        ret(x, b - 82, x + 16, b - 18, marm); ret(x + 11, b - 82, x + 16, b - 18, som);
        for (int s = x + 3; s < x + 14; s += 4) lin(s, b - 80, s, b - 20, som);
        ret(x - 3, b - 86, x + 19, b - 82, marm);
      }
      ret(cx - 160, b - 96, cx + 160, b - 86, marm); ret(cx - 160, b - 90, cx + 160, b - 88, som);
      tri(cx - 166, b - 96, cx, b - 124, cx + 166, b - 96, marm);
      tri(cx - 140, b - 98, cx, b - 118, cx + 140, b - 98, som);
      break;
    }
    case MO_CATEDRAL: {      // catedral gótica: torres com flechas, rosácea, portal
      const RGB888 ped = cr(170, 170, 170), luz = cr(255, 255, 255), som = cr(85, 85, 85);
      ret(cx - 80, b - 80, cx + 80, b, ped); ret(cx + 50, b - 80, cx + 80, b, som);
      tri(cx - 84, b - 80, cx, b - 108, cx + 84, b - 80, ped);
      for (int s = -1; s <= 1; s += 2) {
        int tx = cx + s * 105;
        ret(tx - 25, b - 110, tx + 25, b, ped); ret(tx + 10, b - 110, tx + 25, b, som);
        tri(tx - 27, b - 110, tx, b - 152, tx + 27, b - 110, luz);
        tri(tx, b - 152, tx, b - 110, tx + 27, b - 110, som);
        for (int y = b - 98; y < b - 10; y += 26) { ret(tx - 6, y, tx + 6, y + 16, cr(0, 0, 170)); elip(tx, y, 12, 6, cr(0, 0, 170)); ret(tx - 1, y, tx + 1, y + 16, cr(255, 255, 85)); }
      }
      elip(cx, b - 60, 54, 22, som); elip(cx, b - 60, 46, 18, cr(170, 0, 170));    // rosácea
      for (int k = 0; k < 8; ++k) { double a = k * 0.785; lin(cx, b - 60, cx + (int)(22 * cos(a)), b - 60 + (int)(9 * sin(a)), cr(255, 255, 85)); }
      elip(cx, b - 60, 12, 6, cr(255, 0, 0));
      elip(cx, b - 30, 40, 22, som); ret(cx - 20, b - 30, cx + 20, b, som);           // portal
      ret(cx - 14, b - 26, cx + 14, b, cr(85, 0, 0)); elip(cx, b - 26, 28, 14, cr(85, 0, 0));
      break;
    }
    case MO_OBSERVATORIO: {
      const RGB888 ped = cr(255, 255, 255), som = cr(170, 170, 170);
      ret(cx - 110, b - 50, cx + 110, b, ped); ret(cx + 70, b - 50, cx + 110, b, som);
      for (int x = cx - 96; x < cx + 100; x += 28) { ret(x, b - 40, x + 10, b - 14, cr(255, 255, 85)); elip(x + 5, b - 40, 10, 5, cr(255, 255, 85)); }
      elip(cx, b - 50, 150, 70, som); elip(cx - 6, b - 52, 132, 62, ped);            // cúpula
      ret(cx - 80, b - 50, cx + 80, b - 48, som);
      ret(cx - 5, b - 84, cx + 5, b - 50, cr(0, 0, 85));                              // abertura
      quad(cx - 2, b - 70, cx + 4, b - 74, cx + 40, b - 104, cx + 34, b - 100, cr(170, 85, 0));   // telescópio
      ret(cx + 34, b - 106, cx + 44, b - 100, cr(255, 170, 0));
      break;
    }
    case MO_BARRAGEM: {
      const RGB888 rocha = cr(170, 85, 0), rochaE = cr(85, 0, 0), bet = cr(170, 170, 170), betL = cr(255, 255, 255);
      ret(0, CN_HORIZ - 50, ECRA_W - 1, CN_HORIZ - 40, cr(0, 85, 255));             // lago lá atrás
      quad(0, CN_HORIZ - 70, 210, CN_HORIZ - 40, 230, b, 0, b, rocha);
      quad(ECRA_W - 1, CN_HORIZ - 70, 430, CN_HORIZ - 40, 410, b, ECRA_W - 1, b, rochaE);
      quad(210, CN_HORIZ - 42, 430, CN_HORIZ - 42, 400, b, 240, b, bet);
      quad(210, CN_HORIZ - 42, 260, CN_HORIZ - 42, 280, b, 240, b, betL);
      ret(206, CN_HORIZ - 46, 434, CN_HORIZ - 41, cr(85, 85, 85));                  // estrada no topo
      for (int y = CN_HORIZ - 30; y < b; y += 10) lin(240 + (y - CN_HORIZ) / 6, y, 400 - (y - CN_HORIZ) / 6, y, cr(85, 85, 85));
      break;
    }
    case MO_ARRANHA: {       // arranha-céus de vidro (Nações Unidas / Cura do Cancro)
      const RGB888 vidro = cr(0, 85, 170), vidroL = cr(85, 170, 255), aco = cr(170, 170, 170);
      ret(cx - 220, b - 40, cx - 150, b, cr(85, 85, 85)); ret(cx + 150, b - 56, cx + 230, b, cr(85, 85, 85));
      ret(cx - 60, b - 140, cx + 60, b, vidro); ret(cx + 30, b - 140, cx + 60, b, cr(0, 0, 170));
      for (int y = b - 136; y < b - 4; y += 6) for (int x = cx - 56; x < cx + 56; x += 10) ret(x, y, x + 6, y + 3, vidroL);
      ret(cx - 64, b - 144, cx + 64, b - 140, aco);
      ret(cx - 120, b - 8, cx + 120, b, aco);
      if (w == W_CANCRO) { ret(cx - 6, b - 128, cx + 6, b - 104, cr(255, 0, 0)); ret(cx - 16, b - 120, cx + 16, b - 112, cr(255, 0, 0)); }
      else for (int k = 0; k < 7; ++k) {                                               // bandeiras
        int x = cx - 108 + k * 36; lin(x, b - 34, x, b - 8, aco);
        ret(x + 1, b - 34, x + 14, b - 27, corCiv(1 + k % 7));
      }
      break;
    }
    default: break;
  }
}

// ---------------- estado da cena ----------------
int cnTipo = 0, cnA = 0, cnC = 0, cnFrame = 0, cnDur = 200;
bool cnTerminou = false;
int cnUltimaY = 0;     // revelação: até onde já está pintado

void cenaTextoFinal();
void cenaFundo();
void cenaPasso(int f);

// fanfarra simples (notas tocadas em fotogramas certos)
void notaCena(int f, const int * notas, const int * quando, int n) {
  for (int k = 0; k < n; ++k) if (f == quando[k]) somTri((int)(440.0 * pow(2.0, (notas[k] - 69) / 12.0)), 260, 70);
}

// ---------------- maravilha ----------------
int altoMonumento(int t) {
  switch (t) {
    case MO_PIRAMIDE: return 94; case MO_ZIGURATE: return 92; case MO_ESTATUA: return 150; case MO_FAROL: return 132;
    case MO_TEMPLO: return 126; case MO_CATEDRAL: return 154; case MO_OBSERVATORIO: return 108; case MO_BARRAGEM: return 108;
    case MO_ARRANHA: return 146;
  }
  return 120;
}
void fundoMaravilha() {
  int t = MONUMENTO[cnA];
  switch (t) {
    case MO_PIRAMIDE: case MO_ZIGURATE:
      ceuFaixas(0, CN_HORIZ - 1, CEU_POENTE, 5); sol(470, 70, 10, cr(255, 255, 85), cr(255, 255, 170));
      chao(cr(255, 170, 0), cr(170, 85, 0)); break;
    case MO_ESTATUA: case MO_FAROL:
      ceuFaixas(0, CN_HORIZ - 1, CEU_DIA, 4); sol(100, 30, 8, cr(255, 255, 85), cr(255, 255, 170)); mar(); break;
    case MO_OBSERVATORIO:
      ceuFaixas(0, CN_HORIZ - 1, CEU_NOITE, 3); estrelas(120, 7); elip(90, 30, 30, 14, cr(255, 255, 170));
      chao(cr(0, 85, 0), cr(0, 0, 0)); break;
    case MO_FOGUETAO:
      ceuFaixas(0, CN_HORIZ - 1, CEU_MADRUGADA, 4); estrelas(40, 3); chao(cr(85, 85, 85), cr(0, 0, 0)); break;
    case MO_NAVIO:
      ceuFaixas(0, CN_HORIZ - 1, CEU_DIA, 4); sol(520, 26, 8, cr(255, 255, 85), cr(255, 255, 170)); mar(); break;
    case MO_COGUMELO:
      ceuFaixas(0, CN_HORIZ - 1, CEU_DIA, 4); chao(cr(255, 255, 85), cr(255, 170, 0)); break;
    default:
      ceuFaixas(0, CN_HORIZ - 1, CEU_DIA, 4); sol(90, 26, 8, cr(255, 255, 85), cr(255, 255, 170));
      chao(cr(0, 170, 0), cr(0, 85, 0)); break;
  }
}
// navio (caravela) com o casco a começar em x
void navio(int x, int y) {
  quad(x, y, x + 120, y, x + 104, y + 18, x + 12, y + 18, cr(170, 85, 0));
  ret(x - 4, y - 14, x + 26, y, cr(170, 85, 0)); ret(x - 4, y - 16, x + 26, y - 14, cr(85, 0, 0));
  lin(x + 2, y + 9, x + 112, y + 9, cr(85, 0, 0));
  for (int m = 0; m < 2; ++m) {
    int mx = x + 46 + m * 40;
    ret(mx, y - 70, mx + 2, y, cr(85, 0, 0));
    ret(mx - 18, y - 64, mx + 20, y - 24, cr(255, 255, 255)); ret(mx + 12, y - 64, mx + 20, y - 24, cr(170, 170, 170));
    ret(mx - 2, y - 58, mx + 4, y - 30, cr(255, 0, 0)); ret(mx - 12, y - 48, mx + 14, y - 42, cr(255, 0, 0));
  }
  quad(x + 120, y, x + 150, y - 14, x + 152, y - 12, x + 122, y + 2, cr(85, 0, 0));    // gurupés
}
void foguetao(int cx, int y) {      // y = fundo do foguetão
  ret(cx - 10, y - 70, cx + 10, y, cr(255, 255, 255)); ret(cx + 4, y - 70, cx + 10, y, cr(170, 170, 170));
  for (int k = 0; k < 3; ++k) ret(cx - 10, y - 60 + k * 20, cx + 10, y - 56 + k * 20, cr(0, 0, 0));
  tri(cx - 10, y - 70, cx, y - 92, cx + 10, y - 70, cr(255, 0, 0));
  tri(cx - 10, y - 14, cx - 20, y, cx - 10, y, cr(170, 170, 170)); tri(cx + 10, y - 14, cx + 20, y, cx + 10, y, cr(85, 85, 85));
}
void passoMaravilha(int f) {
  int t = MONUMENTO[cnA];
  static const int nf[6] = { 60, 64, 67, 72, 67, 72 }, qf[6] = { 150, 160, 170, 180, 200, 208 };
  if (t == MO_NAVIO) {
    // o navio entra pela esquerda e pára a meio
    if (f <= 150) {
      int x = -160 + f * 380 / 150, xa = -160 + (f - 1) * 380 / 150;
      recorta(xa - 6, 30, x + 160, CN_FIM_Y);
      ceuFaixas(0, CN_HORIZ - 1, CEU_DIA, 4); sol(520, 26, 8, cr(255, 255, 85), cr(255, 255, 170)); mar();
      navio(x, CN_HORIZ + 8);
      semRecorte();
    }
  } else if (t == MO_FOGUETAO) {
    // a torre e o foguetão; depois o lançamento, com fogo e fumo
    if (f == 1) {
      for (int y = CN_BASE - 110; y < CN_BASE; y += 8) { lin(380, y, 396, y + 8, cr(170, 85, 0)); lin(396, y, 380, y + 8, cr(170, 85, 0)); }
      ret(378, CN_BASE - 110, 380, CN_BASE, cr(170, 85, 0)); ret(396, CN_BASE - 110, 398, CN_BASE, cr(170, 85, 0));
      ret(270, CN_BASE - 4, 420, CN_BASE, cr(170, 170, 170));
    }
    int sobe = f < 60 ? 0 : (f - 60) * (f - 60) / 40;
    int y = CN_BASE - 4 - sobe, ya = CN_BASE - 4 - (f < 61 ? 0 : (f - 61) * (f - 61) / 40);
    if (y > -100) {
      recorta(290, y - 96, 360, ya + 30);
      ceuFaixas(0, CN_HORIZ - 1, CEU_MADRUGADA, 4); estrelas(40, 3);
      if (ya + 30 >= CN_HORIZ) chao(cr(85, 85, 85), cr(0, 0, 0));
      foguetao(320, y);
      if (f >= 50) { elip(320, y + 10, 16, 14 + (f % 3) * 4, cr(255, 85, 0)); elip(320, y + 8, 8, 8, cr(255, 255, 85)); }
      semRecorte();
      if (f >= 55 && f < 160) {        // fumo no chão
        int r = (f - 55) / 2;
        elip(320 - r, CN_BASE - 4, 30 + r, 12 + r / 4, cr(170, 170, 170));
        elip(320 + r, CN_BASE - 6, 26 + r, 10 + r / 4, cr(255, 255, 255));
      }
    }
  } else if (t == MO_COGUMELO) {
    if (f == 20) { ret(0, 0, ECRA_W - 1, CN_FIM_Y, cr(255, 255, 255)); somRuido(300, 80); }
    if (f == 26) fundoMaravilha();
    if (f > 26 && f < 170) {
      int g = f - 26;
      int alt = min(100, g);
      // apaga a nuvem anterior: céu e chão só dentro da zona da nuvem
      { int r0 = min(70, (g - 1) / 2 + 10), alt0 = min(100, g - 1);
        recorta(320 - r0 * 2 - 4, CN_BASE - alt0 - r0 - 4, 320 + r0 * 2 + 4, CN_BASE);
        ceuFaixas(0, CN_HORIZ - 1, CEU_DIA, 4); chao(cr(255, 255, 85), cr(255, 170, 0));
        semRecorte(); }
      ret(312, CN_BASE - alt, 328, CN_BASE, cr(170, 85, 0));
      ret(316, CN_BASE - alt, 324, CN_BASE, cr(255, 170, 0));
      int r = min(70, g / 2 + 10);
      elip(320, CN_BASE - alt, r * 4, r + 10, cr(85, 85, 85));
      elip(320, CN_BASE - alt - 4, r * 3, r, cr(170, 85, 0));
      elip(320, CN_BASE - alt - 6, r * 2, r - 6, cr(255, 170, 0));
      elip(320, CN_BASE - alt - 8, r, r / 2, cr(255, 255, 85));
    }
  } else {
    // os outros monumentos crescem de baixo para cima (ou da esquerda para a direita, a muralha)
    const int dur = 150;
    if (f <= dur) {
      if (t == MO_MURALHA) {
        int x = f * ECRA_W / dur, xa = (f - 1) * ECRA_W / dur;
        recorta(xa, 0, x, CN_FIM_Y);
        desenhaMonumento(t, cnA);
        semRecorte();
      } else {
        int alto = altoMonumento(t) + 8;
        int y = CN_BASE + 2 - alto * f / dur, ya = CN_BASE + 2 - alto * (f - 1) / dur;
        recorta(0, y, ECRA_W - 1, ya);
        desenhaMonumento(t, cnA);
        semRecorte();
        if (f % 12 == 0) som(80 + aleat(40), 20, 30);         // o bater das pedras
      }
    }
    if (f > dur && f < dur + 60 && (t == MO_FAROL)) {          // o farol acende-se
      int k = f - dur;
      lin(320, CN_BASE - 124, 320 - k * 4, CN_BASE - 124 - k / 3, cr(255, 255, 85));
      lin(320, CN_BASE - 124, 320 + k * 4, CN_BASE - 124 - k / 3, cr(255, 255, 85));
    }
  }
  // brilhos à volta quando acaba
  if (f > 160 && f < 210 && f % 5 == 0 && t != MO_COGUMELO) {
    int x = 200 + aleat(240), y = 10 + aleat(60);
    canvas.setPixel(x, y, cr(255, 255, 255)); canvas.setPixel(x - 1, y, cr(255, 255, 85)); canvas.setPixel(x + 1, y, cr(255, 255, 85));
  }
  notaCena(f, nf, qf, 6);
}

// ---------------- conquista ----------------
int cnTipoUnid = UN_LEGIAO;
void cidadeConquista(int corBandeira, int alturaBandeira) {
  // casas por trás das muralhas
  for (int k = 0; k < 7; ++k) {
    int x = 330 + k * 34, y = 70 + (k % 3) * 6;
    ret(x, y, x + 26, y + 30, cr(255, 255, 255)); ret(x + 18, y, x + 26, y + 30, cr(170, 170, 170));
    tri(x - 4, y, x + 13, y - 14, x + 30, y, cr(255, 0, 0)); tri(x + 13, y - 14, x + 30, y, x + 13, y, cr(170, 0, 0));
    ret(x + 6, y + 8, x + 10, y + 14, cr(0, 0, 85));
  }
  // muralha com ameias e porta
  ret(300, 96, 600, 132, cr(170, 170, 170)); ret(300, 96, 600, 99, cr(255, 255, 255));
  for (int x = 300; x < 600; x += 16) ret(x, 90, x + 8, 96, cr(170, 170, 170));
  pedraLinhas(300, 600, 99, 132, 6, cr(85, 85, 85));
  ret(330, 104, 362, 132, cr(85, 0, 0)); elip(346, 104, 32, 12, cr(85, 0, 0));
  // torre com a bandeira
  ret(440, 40, 480, 132, cr(170, 170, 170)); ret(468, 40, 480, 132, cr(85, 85, 85));
  for (int x = 440; x < 480; x += 12) ret(x, 34, x + 6, 40, cr(170, 170, 170));
  ret(459, 2, 461, 34, cr(85, 85, 85));
  if (corBandeira >= 0) { int y = 4 + alturaBandeira; ret(462, y, 492, y + 10, corCiv(corBandeira)); ret(462, y + 10, 492, y + 11, corCivE(corBandeira)); }
}
void fundoConquista() {
  ceuFaixas(0, CN_HORIZ - 1, CEU_DIA, 4);
  sol(80, 24, 8, cr(255, 255, 85), cr(255, 255, 170));
  chao(cr(0, 170, 0), cr(0, 85, 0));
  elip(450, CN_HORIZ + 8, 420, 50, cr(0, 85, 0));
  cidadeConquista(-1, 0);
}
int cnCorAntiga = 0;
void bandeira(int y, int cor) {
  recorta(456, 2, 496, 33);
  ceuFaixas(0, CN_HORIZ - 1, CEU_DIA, 4);
  ret(459, 2, 461, 34, cr(85, 85, 85));
  if (cor >= 0) { ret(462, y, 492, y + 10, corCiv(cor)); ret(462, y + 10, 492, y + 11, corCivE(cor)); }
  semRecorte();
}
void passoConquista(int f) {
  int corNova = corDe(jogador);
  if (f == 1) bandeira(4, cnCorAntiga);
  // as tropas marcham até à porta
  if (f <= 100) {
    recorta(0, 112, 329, 150);
    chao(cr(0, 170, 0), cr(0, 85, 0));
    elip(450, CN_HORIZ + 8, 420, 50, cr(0, 85, 0));
    for (int u = 0; u < 3; ++u) {
      int x = -60 + f * 3 - u * 56, y = 118 + (u % 2) * 8;
      if (x < -50 || x > 280) continue;
      retangulo(x + 1, y + 1, x + TILE_W - 3, y + TILE_H - 2, corCiv(corNova));
      canvas.drawBitmap(x, y, &BMP_UNI[cnTipoUnid]);
    }
    semRecorte();
    if (f % 10 == 0) som(90, 30, 40);       // o tambor da marcha
  }
  // a bandeira deles desce, a nossa sobe
  if (f > 100 && f <= 122) bandeira(4 + (f - 100), cnCorAntiga);
  if (f == 123) bandeira(0, -1);
  if (f > 126 && f <= 148) bandeira(4 + (148 - f), corNova);
  if (f == 150) { somTri(659, 200, 70); }
  if (f == 158) { somTri(784, 360, 70); }
  // fumo e fogo nas casas atacadas
  if (f > 60 && f < 200 && f % 3 == 0) {
    int x = 350 + (f * 37) % 200, y = 60 - (f % 30);
    if (x < 452 || x > 500) elip(x, y, 10 + f % 7, 5, (f / 3) % 2 ? cr(170, 170, 170) : cr(85, 85, 85));
  }
  if (f > 60 && f < 200 && f % 5 == 0) { int x = 340 + (f * 53) % 220; if (x < 436 || x > 484) elip(x, 72, 6, 4, f % 2 ? cr(255, 85, 0) : cr(255, 255, 85)); }
  // o saque: moedas a cair
  if (f > 150 && f < 220 && cnC > 0) {
    int x = 200 + (f * 29) % 120, y = 110 + (f % 5) * 6;
    elip(x, y, 8, 4, cr(255, 255, 85)); elip(x + 1, y, 4, 2, cr(255, 170, 0));
    if (f % 6 == 0) somTri(1568, 30, 40);
  }
}

// ---------------- descoberta (os sábios) ----------------
int eraTec(int t) { return t < 18 ? 0 : t < 38 ? 1 : t < 58 ? 2 : 3; }
void fundoDescoberta() {
  int era = eraTec(cnA);
  const RGB888 parede[4] = { cr(170, 85, 0), cr(85, 0, 0), cr(0, 85, 85), cr(85, 85, 85) };
  ret(0, 0, ECRA_W - 1, CN_FIM_Y, parede[era]);
  for (int y = 6; y < 110; y += 12) for (int x = (y / 12 % 2) * 20; x < ECRA_W; x += 40) ret(x, y, x + 1, y + 1, cr(0, 0, 0));
  ret(0, 110, ECRA_W - 1, CN_FIM_Y, cr(85, 0, 0));                                     // chão de madeira
  for (int y = 114; y <= CN_FIM_Y; y += 5) lin(0, y, ECRA_W - 1, y, cr(170, 85, 0));
  // janela com o céu
  ret(60, 20, 160, 80, cr(85, 85, 85)); ret(64, 24, 156, 76, cr(85, 170, 255)); ret(64, 60, 156, 76, cr(0, 170, 0));
  ret(108, 24, 112, 76, cr(85, 85, 85)); ret(64, 48, 156, 51, cr(85, 85, 85));
  // estante com livros (ou rolos, ou um quadro de circuitos)
  ret(470, 16, 600, 108, cr(85, 0, 0));
  for (int s = 0; s < 4; ++s) {
    int y = 22 + s * 22;
    ret(474, y + 18, 596, y + 20, cr(170, 85, 0));
    for (int x = 476; x < 594; x += 7) {
      RGB888 c = (x / 7) % 4 == 0 ? cr(170, 0, 0) : (x / 7) % 4 == 1 ? cr(0, 0, 170) : (x / 7) % 4 == 2 ? cr(0, 85, 0) : cr(170, 170, 0);
      if (era == 0) elip(x + 3, y + 14, 6, 4, cr(255, 255, 170)); else ret(x, y + 4 + (x % 3), x + 5, y + 18, c);
    }
  }
  // mesa
  ret(220, 96, 440, 102, cr(170, 85, 0)); ret(226, 102, 234, 140, cr(85, 0, 0)); ret(426, 102, 434, 140, cr(85, 0, 0));
  // o sábio (sentado, a olhar para a mesa)
  const RGB888 veste[4] = { cr(255, 255, 255), cr(85, 0, 85), cr(0, 0, 0), cr(255, 255, 255) };
  quad(250, 140, 300, 140, 296, 84, 254, 84, veste[era]);
  quad(254, 84, 296, 84, 290, 68, 260, 68, veste[era]);
  elip(275, 58, 26, 14, cr(255, 170, 85));                                              // cabeça
  if (era < 2) { elip(275, 66, 18, 8, cr(255, 255, 255)); ret(266, 46, 284, 50, cr(255, 255, 255)); }   // barba e cabelo brancos
  else ret(262, 46, 288, 50, cr(85, 0, 0));
  canvas.setPixel(282, 57, cr(0, 0, 0));
  quad(292, 80, 300, 78, 330, 92, 322, 96, veste[era]);                                // braço sobre a mesa
  // o que está na mesa
  switch (era) {
    case 0: ret(330, 90, 380, 95, cr(255, 255, 170)); elip(330, 92, 6, 6, cr(255, 170, 85)); elip(380, 92, 6, 6, cr(255, 170, 85)); break;   // rolo
    case 1: ret(320, 86, 370, 95, cr(255, 255, 255)); ret(344, 86, 346, 95, cr(170, 170, 170)); lin(372, 70, 362, 90, cr(255, 255, 255)); break;   // livro e pena
    case 2: ret(330, 76, 340, 95, cr(85, 255, 85)); elip(335, 76, 12, 6, cr(85, 255, 85)); ret(360, 70, 366, 95, cr(255, 85, 255)); elip(400, 82, 26, 14, cr(0, 85, 255)); ret(398, 92, 402, 95, cr(170, 85, 0)); break;   // frascos e globo
    case 3: ret(330, 60, 400, 92, cr(85, 85, 85)); ret(334, 64, 396, 88, cr(0, 85, 0)); for (int y = 68; y < 86; y += 4) ret(338, y, 338 + (y * 13) % 50, y + 1, cr(85, 255, 85)); ret(350, 92, 380, 95, cr(170, 170, 170)); break;   // computador
  }
  // vela ou candeeiro
  if (era < 3) { ret(420, 80, 426, 95, cr(255, 255, 255)); }
}
void passoDescoberta(int f) {
  int era = eraTec(cnA);
  // a chama da vela treme
  if (era < 3 && f < 200) { ret(418, 66, 428, 79, eraTec(cnA) == 0 ? cr(170, 85, 0) : (era == 1 ? cr(85, 0, 0) : cr(0, 85, 85))); elip(423, 74 - (f / 4) % 2, 6, 8, cr(255, 170, 0)); elip(423, 76, 2, 3, cr(255, 255, 85)); }
  // a ideia: uma luz por cima da cabeça, com raios
  if (f == 70) { somTri(523, 120, 60); }
  if (f == 76) { somTri(659, 120, 60); }
  if (f == 82) { somTri(784, 220, 70); }
  if (f >= 70 && f < 110) {
    int k = f - 70;
    elip(275, 22, 14, 10, cr(255, 255, 85)); elip(275, 22, 6, 4, cr(255, 255, 255));
    for (int a = 0; a < 8; ++a) {
      double an = a * 0.785;
      int r = 10 + k / 2;
      lin(275 + (int)(r * 1.6 * cos(an)), 22 + (int)(r * 0.6 * sin(an)), 275 + (int)((r + 6) * 1.6 * cos(an)), 22 + (int)((r + 6) * 0.6 * sin(an)), cr(255, 255, 85));
    }
  }
}

// ---------------- o texto no fim ----------------
void cenaTextoFinal() {
  char a[120], b[120];
  a[0] = b[0] = 0;
  switch (cnTipo) {
    case CN_MARAVILHA:
      snprintf(a, sizeof(a), "Os %s construíram: %s, em %s!", nomePovo(jogador), MARAVILHA[cnA].nome, CID[cnC].nome);
      snprintf(b, sizeof(b), "%s", MARAVILHA[cnA].efeito);
      break;
    case CN_CONQUISTA:
      snprintf(a, sizeof(a), "Conquistámos %s! A cidade passa a ser dos %s.", CID[cnA].nome, nomePovo(jogador));
      if (cnC > 0) snprintf(b, sizeof(b), "Saque: %d moedas de ouro.", cnC); else snprintf(b, sizeof(b), "Não havia ouro para saquear.");
      break;
    case CN_DESCOBERTA:
      snprintf(a, sizeof(a), "Os nossos sábios fizeram uma descoberta: %s!", TEC[cnA].nome);
      snprintf(b, sizeof(b), "Carrega em ESPAÇO para saberes mais.");
      break;
  }
  caixa(4, 158, ECRA_W - 5, 197, C_PAINEL, C_MOLDURA);
  textoCentro(4, ECRA_W - 5, 163, a, C_AMARELO);
  textoCentro(4, ECRA_W - 5, 175, b, C_BRANCO);
  textoDireita(ECRA_W - 12, 187, "ESPAÇO", C_CINZA);
  moldura();
}

// ---------------- motor ----------------
void cenaFundo() {
  switch (cnTipo) { case CN_MARAVILHA: fundoMaravilha(); break; case CN_CONQUISTA: fundoConquista(); break; case CN_DESCOBERTA: fundoDescoberta(); break; }
  ret(0, CN_FIM_Y + 1, ECRA_W - 1, ECRA_H - 1, cr(0, 0, 0));
  moldura();
}
void cenaPasso(int f) {
  switch (cnTipo) { case CN_MARAVILHA: passoMaravilha(f); break; case CN_CONQUISTA: passoConquista(f); break; case CN_DESCOBERTA: passoDescoberta(f); break; }
}
// salta para o fim: desenha o estado final de uma vez
void cenaSaltaFim() {
  cenaFundo();
  switch (cnTipo) {
    case CN_MARAVILHA: {
      int t = MONUMENTO[cnA];
      if (t == MO_NAVIO) navio(220, CN_HORIZ + 8);
      else if (t == MO_FOGUETAO) { for (int y = CN_BASE - 110; y < CN_BASE; y += 8) { lin(380, y, 396, y + 8, cr(170, 85, 0)); lin(396, y, 380, y + 8, cr(170, 85, 0)); } elip(320, 60, 200, 60, cr(170, 170, 170)); foguetao(320, 30); }
      else if (t == MO_COGUMELO) passoMaravilha(169);
      else desenhaMonumento(t, cnA);
      break;
    }
    case CN_CONQUISTA: {
      cidadeConquista(corDe(jogador), 0);
      for (int u = 0; u < 3; ++u) { int x = 240 - u * 56, y = 118 + (u % 2) * 8; retangulo(x + 1, y + 1, x + TILE_W - 3, y + TILE_H - 2, corCiv(corDe(jogador))); canvas.drawBitmap(x, y, &BMP_UNI[cnTipoUnid]); }
      break;
    }
    case CN_DESCOBERTA: passoDescoberta(90); break;
  }
}

void abreCena(int tipo, int a, int c, int b) {
  cnTipo = tipo; cnA = a; cnC = c; cnFrame = 0; cnTerminou = false;
  cnDur = tipo == CN_DESCOBERTA ? 140 : 230;
  if (tipo == CN_CONQUISTA) {
    cnCorAntiga = b;                          // a cor de quem tinha a cidade
    // a unidade que entrou na cidade
    cnTipoUnid = UN_LEGIAO;
    int lista[16], n = unidadesEm(CID[a].x, CID[a].y, lista, 16);
    for (int k2 = 0; k2 < n; ++k2) if (U[lista[k2]].civ == jogador) { cnTipoUnid = U[lista[k2]].tipo; break; }
  }
  entraModo(M_CENA);
  semRecorte();
  cenaFundo();
}
void atualizaCena() {
  if (!cnTerminou) {
    ++cnFrame;
    cenaPasso(cnFrame);
    bool salta = tempo > 10 && carregouFogo();
    if (cnFrame >= cnDur || salta) {
      if (salta) cenaSaltaFim();
      cnTerminou = true;
      semRecorte();
      cenaTextoFinal();
      tempo = 0;
    }
    return;
  }
  if (tempo > 8 && (carregouFogo() || carregouVolta())) {
    somOk();
    if (cnTipo == CN_DESCOBERTA) {            // a seguir, a janela da descoberta com o texto da enciclopédia
      abreDescoberta(cnA, 1);
      return;
    }
    entraModo(M_MAPA);
    desenhaTudo();
    continuaTurno();
  }
}
