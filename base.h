// =====================================================================
// base.h — IMPÉRIOS: entrada (teclado e joystick), som, música e texto.
// (vem da Coroa de 1580, adaptado a 640x200)
// =====================================================================
#pragma once

// ---------------- cores (paleta de 64 cores: 0, 85, 170, 255) ----------------
const RGB888 C_PRETO    = RGB888(0, 0, 0);
const RGB888 C_BRANCO   = RGB888(255, 255, 255);
const RGB888 C_AMARELO  = RGB888(255, 255, 85);
const RGB888 C_OURO     = RGB888(255, 170, 0);
const RGB888 C_CINZA    = RGB888(170, 170, 170);
const RGB888 C_CINZA_E  = RGB888(85, 85, 85);
const RGB888 C_VERMELHO = RGB888(255, 85, 85);
const RGB888 C_VERM_E   = RGB888(170, 0, 0);
const RGB888 C_VERDE    = RGB888(85, 255, 85);
const RGB888 C_VERDE_E  = RGB888(0, 170, 0);
const RGB888 C_AZUL     = RGB888(85, 85, 255);
const RGB888 C_AZUL_E   = RGB888(0, 0, 170);
const RGB888 C_CEU      = RGB888(85, 170, 255);
const RGB888 C_RIO      = RGB888(0, 85, 255);
const RGB888 C_COSTA    = RGB888(85, 170, 255);
const RGB888 C_ESTRADA  = RGB888(170, 85, 0);
const RGB888 C_PAINEL   = RGB888(0, 0, 85);
const RGB888 C_MOLDURA  = RGB888(170, 85, 0);
const RGB888 C_FUNDO    = RGB888(0, 0, 0);

RGB888 corCiv(int cor)  { const InfoCor & c = COR_CIV[cor]; return RGB888(c.r, c.g, c.b); }
RGB888 corCivE(int cor) { const InfoCor & c = COR_CIV[cor]; return RGB888(c.re, c.ge, c.be); }

// ---------------- entrada ----------------
fabgl::Keyboard * kb = nullptr;
bool temJoystick = false;
uint8_t joy = 0;

struct Botoes { bool cima, baixo, esq, dir, fogo, volta; };
Botoes bt, btAntes;
int repeticao[4] = { 0, 0, 0, 0 };

bool tecla(VirtualKey k) { return kb && kb->isVKDown(k); }

uint8_t leJoystick() {
  if (!temJoystick) return 0;
  Wire.beginTransmission(0x20);
  Wire.write(0x12);                                   // GPIOA = joystick 1
  if (Wire.endTransmission(false) != 0) return 0;
  if (Wire.requestFrom((uint8_t)0x20, (uint8_t)1) != 1) return 0;
  return Wire.read();
}

// teclas de letra (atalhos): guardadas pela ordem em que foram premidas
const int MAX_TECLAS = 8;
VirtualKey filaTeclas[MAX_TECLAS];
int nTeclas = 0;

void leEntrada() {
  btAntes = bt;
  kb = PS2.keyboard();
  joy = leJoystick();
  bt.cima  = tecla(VK_UP)    || tecla(VK_KP_UP)    || (joy & 0x01);
  bt.baixo = tecla(VK_DOWN)  || tecla(VK_KP_DOWN)  || (joy & 0x02);
  bt.esq   = tecla(VK_LEFT)  || tecla(VK_KP_LEFT)  || (joy & 0x04);
  bt.dir   = tecla(VK_RIGHT) || tecla(VK_KP_RIGHT) || (joy & 0x08);
  bt.fogo  = tecla(VK_SPACE) || tecla(VK_RETURN) || tecla(VK_KP_ENTER) || (joy & 0x10);
  bt.volta = tecla(VK_ESCAPE) || tecla(VK_BACKSPACE) || (joy & 0x20);
  // teclas de atalho (letras, números, teclado numérico): vêm da fila do teclado
  nTeclas = 0;
  if (kb) {
    while (kb->virtualKeyAvailable() && nTeclas < MAX_TECLAS) {
      bool baixo = false;
      VirtualKey k = kb->getNextVirtualKey(&baixo);
      if (baixo) filaTeclas[nTeclas++] = k;
    }
  }
}

bool carregouFogo()  { return bt.fogo && !btAntes.fogo; }
bool carregouVolta() { return bt.volta && !btAntes.volta; }
bool premiu(VirtualKey k) { for (int i = 0; i < nTeclas; ++i) if (filaTeclas[i] == k) return true; return false; }
bool premiuLetra(char c) {
  // aceita maiúscula e minúscula
  for (int i = 0; i < nTeclas; ++i) {
    int a = kb ? kb->virtualKeyToASCII(filaTeclas[i]) : -1;
    if (a >= 0 && (a == c || a == c + 32 || a == c - 32)) return true;
  }
  return false;
}

// direções com repetição automática (para os menus e o cursor do mapa)
bool repete(int i, bool agora) {
  if (!agora) { repeticao[i] = 0; return false; }
  ++repeticao[i];
  return repeticao[i] == 1 || (repeticao[i] > 16 && (repeticao[i] - 16) % 5 == 0);
}
bool menuCima()  { return repete(0, bt.cima); }
bool menuBaixo() { return repete(1, bt.baixo); }
bool menuEsq()   { return repete(2, bt.esq); }
bool menuDir()   { return repete(3, bt.dir); }

// ---------------- som ----------------
void som(int freq, int ms, int vol = 30)    { soundGenerator.playSound(SquareWaveformGenerator(), freq, ms, vol); }
void somRuido(int ms, int vol = 60)         { soundGenerator.playSound(NoiseWaveformGenerator(), 600, ms, vol); }
void somTri(int freq, int ms, int vol = 60) { soundGenerator.playSound(TriangleWaveformGenerator(), freq, ms, vol); }
void somMenu()   { som(880, 25, 18); }
void somOk()     { som(1320, 40, 22); }
void somErro()   { som(160, 120, 30); }
void somPasso()  { som(220, 15, 12); }

// ---------------- música (2 vozes, melodias originais) ----------------
struct Nota { uint8_t n, d; };     // nota MIDI (0 = pausa), duração em semicolcheias

const Nota MUS_TITULO[] = {
  {67,4},{72,4},{74,2},{76,2},{77,4},{76,4},{74,4},{72,8},{0,4},
  {72,4},{76,4},{79,2},{77,2},{76,4},{74,4},{72,4},{74,12},{0,4},
  {76,4},{77,2},{79,2},{81,4},{79,4},{77,4},{76,4},{74,8},
  {72,2},{74,2},{76,4},{74,4},{71,4},{72,16},{0,4} };
const Nota BAI_TITULO[] = { {48,8},{53,8},{55,8},{48,8},{45,8},{41,8},{43,16},
                            {48,8},{53,8},{55,8},{50,8},{48,8},{43,8},{48,16} };
const Nota MUS_VITORIA[] = {
  {72,2},{72,1},{72,1},{79,6},{77,2},{76,2},{74,2},{72,4},{79,4},{84,12},{0,4},
  {81,2},{79,2},{77,2},{76,2},{77,4},{74,4},{72,12},{0,4} };
const Nota BAI_VITORIA[] = { {48,8},{43,8},{48,8},{55,8},{53,8},{43,8},{48,16} };
const Nota MUS_DERROTA[] = {
  {69,6},{68,2},{69,4},{64,4}, {65,6},{64,2},{62,8}, {64,6},{62,2},{60,4},{59,4}, {57,16} };
const Nota BAI_DERROTA[] = { {45,16},{38,16},{40,16},{33,16} };

SquareWaveformGenerator   vozMelodia;
TriangleWaveformGenerator vozBaixo;
const Nota * musM = nullptr; int musMn = 0, musMi = 0; uint32_t musMfim = 0;
const Nota * musB = nullptr; int musBn = 0, musBi = 0; uint32_t musBfim = 0;
int  musUnidade = 110;
bool musicaLigada = false, musicaPermitida = true, musicaRepete = true;
const Nota * musAtual = nullptr;

int freqMidi(int n) { return (int)(440.0f * powf(2.0f, (n - 69) / 12.0f)); }

void tocaMusica(const Nota * m, int nm, const Nota * b, int nb, int unidade, bool repete) {
  if (musAtual == m && musicaLigada) return;
  musAtual = m;
  musM = m; musMn = nm; musMi = 0; musB = b; musBn = nb; musBi = 0;
  musUnidade = unidade; musicaRepete = repete;
  musMfim = musBfim = millis();
  musicaLigada = true;
}

void paraMusica() {
  musicaLigada = false;
  musAtual = nullptr;
  vozMelodia.enable(false);
  vozBaixo.enable(false);
}

void atualizaMusica(uint32_t agora) {
  if (!musicaLigada || !musicaPermitida) { vozMelodia.enable(false); vozBaixo.enable(false); return; }
  if ((int32_t)(agora - musMfim) >= 0) {
    if (musMi >= musMn) {
      if (!musicaRepete) { paraMusica(); return; }
      musMi = 0;
    }
    const Nota & n = musM[musMi];
    if (n.n) { vozMelodia.setFrequency(freqMidi(n.n)); vozMelodia.enable(true); }
    else vozMelodia.enable(false);
    musMfim += n.d * musUnidade;
    ++musMi;
  }
  if ((int32_t)(agora - musBfim) >= 0) {
    if (musBi >= musBn) musBi = 0;
    const Nota & n = musB[musBi];
    if (n.n) { vozBaixo.setFrequency(freqMidi(n.n)); vozBaixo.enable(true); }
    else vozBaixo.enable(false);
    musBfim += n.d * musUnidade;
    ++musBi;
  }
}

#define TOCA(m, b, u, r) tocaMusica(m, sizeof(m) / sizeof(Nota), b, sizeof(b) / sizeof(Nota), u, r)

// =====================================================================
// TEXTO: fonte 8x8 da FabGL com ã e õ acrescentados; os textos estão em
// UTF-8 no código e são convertidos para a página de código da fonte.
// =====================================================================
uint8_t  fonteDados[256 * 8];
FontInfo fonte;
const uint8_t CH_ATIL = 1, CH_OTIL = 2;

void preparaFonte() {
  memcpy(fonteDados, fabgl::FONT_8x8.data, 256 * 8);
  const uint8_t * til = fabgl::FONT_8x8.data + '~' * 8;
  int t0 = 0;
  while (t0 < 6 && til[t0] == 0) ++t0;
  for (int r = 0; r < 8; ++r) {
    fonteDados[CH_ATIL * 8 + r] = fabgl::FONT_8x8.data['a' * 8 + r];
    fonteDados[CH_OTIL * 8 + r] = fabgl::FONT_8x8.data['o' * 8 + r];
  }
  for (int r = 0; r < 2; ++r) {
    fonteDados[CH_ATIL * 8 + r] = til[t0 + r];
    fonteDados[CH_OTIL * 8 + r] = til[t0 + r];
  }
  fonte = fabgl::FONT_8x8;
  fonte.data = fonteDados;
}

// converte UTF-8 para a fonte (página de código 437 + ã õ); devolve o nº de letras
int converte(const char * s, char * out, int max) {
  int n = 0;
  while (*s && n < max - 1) {
    uint8_t c = (uint8_t)*s++;
    if (c < 0x80) { out[n++] = (char)c; continue; }
    uint8_t c2 = (uint8_t)*s;
    if (c2) ++s;
    uint8_t r = '?';
    if (c == 0xC3) {
      switch (c2) {
        case 0xA1: r = 0xA0; break;   // á
        case 0xA0: r = 0x85; break;   // à
        case 0xA2: r = 0x83; break;   // â
        case 0xA3: r = CH_ATIL; break;// ã
        case 0xA9: r = 0x82; break;   // é
        case 0xAA: r = 0x88; break;   // ê
        case 0xAD: r = 0xA1; break;   // í
        case 0xB3: r = 0xA2; break;   // ó
        case 0xB4: r = 0x93; break;   // ô
        case 0xB5: r = CH_OTIL; break;// õ
        case 0xBA: r = 0xA3; break;   // ú
        case 0xBC: r = 0x81; break;   // ü
        case 0xA7: r = 0x87; break;   // ç
        case 0x87: r = 0x80; break;   // Ç
        case 0x89: r = 0x90; break;   // É
        case 0x81: case 0x80: case 0x82: case 0x83: r = 'A'; break;
        case 0x8D: r = 'I'; break;
        case 0x93: case 0x94: case 0x95: r = 'O'; break;
        case 0x9A: r = 'U'; break;
        default: r = '?';
      }
    } else if (c == 0xC2 && c2 == 0xBA) r = 0xA7;          // º
    else if (c == 0xE2) { if (*s) ++s; r = '-'; }          // travessão e afins
    out[n++] = (char)r;
  }
  out[n] = 0;
  return n;
}

int letras(const char * s) { char b[200]; return converte(s, b, sizeof(b)); }

void texto(int x, int y, const char * s, RGB888 cor) {
  char b[200];
  converte(s, b, sizeof(b));
  canvas.setPenColor(cor);
  canvas.drawText(&fonte, x, y, b);
}

void textoF(int x, int y, RGB888 cor, const char * fmt, ...) {
  char b[200];
  va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof(b), fmt, ap); va_end(ap);
  texto(x, y, b, cor);
}

void textoCentro(int x0, int x1, int y, const char * s, RGB888 cor) {
  int n = letras(s);
  texto(x0 + ((x1 - x0 + 1) - n * 8) / 2, y, s, cor);
}

// texto encostado à direita (termina em x1)
void textoDireita(int x1, int y, const char * s, RGB888 cor) {
  texto(x1 + 1 - letras(s) * 8, y, s, cor);
}

// escreve com quebras de linha nas palavras; devolve a altura usada
int textoQuebrado(int x, int y, int maxLetras, const char * s, RGB888 cor, int entreLinhas = 9) {
  char b[600];
  int n = converte(s, b, sizeof(b));
  int ini = 0, linhas = 0;
  canvas.setPenColor(cor);
  while (ini < n) {
    while (ini < n && b[ini] == ' ') ++ini;
    if (ini >= n) break;
    int fim = ini + maxLetras;
    if (fim >= n) fim = n;
    else {
      int k = fim;
      while (k > ini && b[k] != ' ') --k;
      if (k > ini) fim = k;
    }
    for (int i = ini; i < fim; ++i) if (b[i] == '\n') { fim = i; break; }
    char l[100];
    int len = max(0, min(fim - ini, 99));
    memcpy(l, b + ini, len); l[len] = 0;
    canvas.drawText(&fonte, x, y + linhas * entreLinhas, l);
    ++linhas;
    ini = fim;
    if (ini < n && b[ini] == '\n') ++ini;
  }
  return linhas * entreLinhas;
}

// conta quantas linhas um texto ocupa com quebras
int linhasTexto(int maxLetras, const char * s) {
  char b[600];
  int n = converte(s, b, sizeof(b));
  int ini = 0, linhas = 0;
  while (ini < n) {
    while (ini < n && b[ini] == ' ') ++ini;
    if (ini >= n) break;
    int fim = ini + maxLetras;
    if (fim >= n) fim = n;
    else { int k = fim; while (k > ini && b[k] != ' ') --k; if (k > ini) fim = k; }
    for (int i = ini; i < fim; ++i) if (b[i] == '\n') { fim = i; break; }
    ++linhas; ini = fim;
    if (ini < n && b[ini] == '\n') ++ini;
  }
  return linhas;
}

// letras grandes (título): cada píxel da fonte vira um retângulo esc x escY
void textoGrande(int x, int y, const char * s, RGB888 cor, int esc, int escY, const FontInfo * f) {
  char b[64];
  converte(s, b, sizeof(b));
  canvas.setBrushColor(cor);
  for (int i = 0; b[i]; ++i) {
    uint8_t ch = (uint8_t)b[i];
    const uint8_t * g; int w;
    if (f->chptr) { g = f->data + f->chptr[ch]; w = *g++; }
    else { w = f->width; g = f->data + ch * f->height * ((f->width + 7) / 8); }
    int bpr = (w + 7) / 8;
    for (int r = 0; r < f->height; ++r) {
      int x0 = -1;
      for (int c = 0; c <= w; ++c) {
        bool on = c < w && ((g[r * bpr + (c >> 3)] >> (7 - (c & 7))) & 1);
        if (on && x0 < 0) x0 = c;
        if (!on && x0 >= 0) {
          canvas.fillRectangle(x + x0 * esc, y + r * escY, x + c * esc - 1, y + r * escY + escY - 1);
          x0 = -1;
        }
      }
    }
    x += w * esc;
  }
}

void caixa(int x0, int y0, int x1, int y1, RGB888 fundo, RGB888 borda) {
  canvas.setBrushColor(fundo);
  canvas.fillRectangle(x0, y0, x1, y1);
  canvas.setPenColor(borda);
  canvas.drawRectangle(x0, y0, x1, y1);
  canvas.drawRectangle(x0 + 2, y0 + 1, x1 - 2, y1 - 1);
}

void retangulo(int x0, int y0, int x1, int y1, RGB888 cor) {
  canvas.setBrushColor(cor);
  canvas.fillRectangle(x0, y0, x1, y1);
}

void limpaEcra(RGB888 cor) {
  canvas.setBrushColor(cor);
  canvas.clear();
}

// desenha um bitmap RGBA2222 da memória de programa (casa de 32x14)
Bitmap bmpTmp;
void desenhaArte(int x, int y, const uint8_t * dados, int w = 32, int h = 14) {
  bmpTmp = Bitmap(w, h, dados, PixelFormat::RGBA2222);
  canvas.drawBitmap(x, y, &bmpTmp);
  // a FabGL guarda o ponteiro do bitmap até ao desenho: esperar que acabe antes de reutilizar
  canvas.waitCompletion(false);
}
