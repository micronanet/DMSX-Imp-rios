// =====================================================================
// jogo.h — IMPÉRIOS: o jogo (modos, turno do jogador, menus, cidade,
// relatórios, gravação, fim)
// =====================================================================
#pragma once

enum Modo : uint8_t { M_TITULO, M_LISTA, M_MAPA, M_MENSAGEM, M_PERGUNTA, M_CIDADE, M_RELATORIO, M_DESTINO, M_TAXAS, M_FIM, M_ESPERA, M_ENCONTRO, M_DESCOBERTA, M_ATAQUE, M_NOME, M_MUNDO, M_CENA };
Modo modo = M_TITULO;
uint32_t tempo = 0, frame = 0;
void entraModo(Modo m) { modo = m; tempo = 0; }

// o que cada lista está a escolher
enum IdLista : uint8_t { L_TITULO, L_DIFICULDADE, L_NCIVS, L_POVO, L_PRODUCAO, L_PESQUISA, L_ORDENS, L_MENU,
                         L_GOVERNO, L_CIDADES, L_DIPLOMATA, L_CARAVANA, L_OPCOES_CIDADE, L_DIPLOMACIA, L_DIPLO_ACAO, L_VENDER, L_UNID_CIDADE, L_ALVOS, L_IR_CIDADE, L_GRAVAR, L_CARREGAR, L_DORMIR };
Modo modoAntesLista = M_MAPA;
bool podeMarchar(int i);
void abreAlvos();
void abreAtaque(int i, int ci);
void abreGravar();
void abreCarregar();
void abreNome();
void abreNomeCidade(int ci);
void abreDormir();
void abreMundo();
void abreCena(int tipo, int a, int c, int b);   // cenas.h
uint32_t sementeEscolhida = 1;
// unidades que chegaram junto de uma cidade inimiga (marcha com a tecla A)
const int MAX_CHEGADAS = 8;
int chegouUnid[MAX_CHEGADAS], chegouCid[MAX_CHEGADAS], nChegadas = 0;

int cidadeAtual = -1;             // cidade aberta (ecrã da cidade) ou a escolher produção
int alvoX = 0, alvoY = 0;          // diplomata/caravana: a cidade onde entrou
int civDiplomacia = 0;

// ---------------- gravação no cartão SD ----------------
// ---------------- gravações no cartão SD (10 posições, como no original) ----------------
// cada gravação tem um cabeçalho com o nome do jogador, o povo, o ano e a dificuldade
const int N_GRAVACOES = 10;
const int GRAV_AUTO = -1, GRAV_ANTIGA = N_GRAVACOES;     // gravação automática; ficheiro da versão anterior
const uint32_t MAGICO = 0x31504D49;    // "IMP1": versão anterior, sem cabeçalho
const uint32_t MAGICO2 = 0x32504D49;   // "IMP2": com cabeçalho (mapa 80x50)
const uint32_t MAGICO3 = 0x33504D49;   // "IMP3": cabeçalho com o tamanho do mapa e as opções do mundo
bool temSD = false;
char nomeJogador[24] = "";
bool verInimigos = false;              // opção: mostrar os movimentos das unidades dos outros povos que estão à vista
int ultimaGravacao = -1;               // F10 grava na mesma posição
struct Cabecalho { char nome[24]; uint8_t povo, dificuldade, nCivs, livre; int32_t turno; };
struct Cabecalho3 { uint8_t mapaW, mapaH; OpcoesMundo mundo; };

const char * nomeLider(int c) { return (c == jogador && nomeJogador[0]) ? nomeJogador : POVO[CIV[c].povo].lider; }
void nomeFicheiro(char * b, int k) {
  if (k == GRAV_AUTO) strcpy(b, "/impauto.sav");
  else if (k == GRAV_ANTIGA) strcpy(b, "/imperios.sav");
  else sprintf(b, "/imperio%d.sav", k);
}

// monta o cartão SD (no seu SPI próprio, HSPI); se falhar, volta a tentar da próxima vez que for preciso
bool montaSD() {
  if (temSD) return true;
  static bool spiPronto = false;
  if (!spiPronto) { spiSD.begin(14, 2, 12, 13); spiPronto = true; }
  // só 2 ficheiros abertos de cada vez: gasta menos RAM interna (o VGA a 640x200 já ocupa 128 KB)
  temSD = SD.begin(13, spiSD, 4000000, "/sd", 2);
  Serial.printf("IMPERIOS: cartao SD %s (RAM interna livre %u, maior bloco %u)\n", temSD ? "montado" : "NAO montado",
                (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL), (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
  return temSD;
}

void escreveDados(File & f) {
  f.write((uint8_t *)&turno, sizeof(turno));
  f.write(&jogador, 1); f.write(&dificuldade, 1); f.write(&nCivsJogo, 1); f.write(&fimDeJogo, 1);
  f.write((uint8_t *)&sementeMapa, sizeof(sementeMapa));
  f.write((uint8_t *)maravilhaCidade, sizeof(maravilhaCidade));
  f.write(tecDescoberta, sizeof(tecDescoberta));
  f.write((uint8_t *)CIV, sizeof(CIV));
  f.write(mapaTer, MAPA_N); f.write(mapaMelh, MAPA_N); f.write(mapaVis, MAPA_N); f.write(mapaCont, MAPA_N);
  f.write((uint8_t *)U, sizeof(Unidade) * MAX_UNIDADES);
  f.write((uint8_t *)CID, sizeof(Cidade) * MAX_CIDADES);
}
void leDados(File & f) {
  f.read((uint8_t *)&turno, sizeof(turno));
  f.read(&jogador, 1); f.read(&dificuldade, 1); f.read(&nCivsJogo, 1); f.read(&fimDeJogo, 1);
  f.read((uint8_t *)&sementeMapa, sizeof(sementeMapa));
  f.read((uint8_t *)maravilhaCidade, sizeof(maravilhaCidade));
  f.read(tecDescoberta, sizeof(tecDescoberta));
  f.read((uint8_t *)CIV, sizeof(CIV));
  f.read(mapaTer, MAPA_N); f.read(mapaMelh, MAPA_N); f.read(mapaVis, MAPA_N); f.read(mapaCont, MAPA_N);
  f.read((uint8_t *)U, sizeof(Unidade) * MAX_UNIDADES);
  f.read((uint8_t *)CID, sizeof(Cidade) * MAX_CIDADES);
}

bool gravaJogo(int k) {
  if (!montaSD()) return false;
  char nf[20]; nomeFicheiro(nf, k);
  File f = SD.open(nf, FILE_WRITE);
  if (!f) return false;
  uint32_t m = MAGICO3;
  f.write((uint8_t *)&m, 4);
  Cabecalho c; memset(&c, 0, sizeof(c));
  snprintf(c.nome, sizeof(c.nome), "%s", nomeLider(jogador));
  c.povo = CIV[jogador].povo; c.dificuldade = dificuldade; c.nCivs = nCivsJogo; c.turno = turno;
  c.livre = verInimigos ? 1 : 0;                  // opções do jogador (bit 0: ver os movimentos dos outros)
  f.write((uint8_t *)&c, sizeof(c));
  Cabecalho3 c3 = { (uint8_t)MAPA_W, (uint8_t)MAPA_H, mundo };
  f.write((uint8_t *)&c3, sizeof(c3));
  escreveDados(f);
  f.close();
  if (k != GRAV_AUTO) ultimaGravacao = k;
  return true;
}

// lê só o cabeçalho (para as listas de gravações)
bool leCabecalho(int k, Cabecalho & c) {
  if (!montaSD()) return false;
  char nf[20]; nomeFicheiro(nf, k);
  if (!SD.exists(nf)) return false;
  File f = SD.open(nf, FILE_READ);
  if (!f) return false;
  uint32_t m = 0;
  f.read((uint8_t *)&m, 4);
  bool ok = true;
  if (m == MAGICO2 || m == MAGICO3) f.read((uint8_t *)&c, sizeof(c));
  else if (m == MAGICO) {
    // versão anterior: tira o que puder dos dados
    memset(&c, 0, sizeof(c));
    uint8_t jog = 0;
    f.read((uint8_t *)&c.turno, sizeof(turno));
    f.read(&jog, 1); f.read(&c.dificuldade, 1); f.read(&c.nCivs, 1);
    size_t civ0 = 4 + sizeof(turno) + 4 + sizeof(sementeMapa) + sizeof(maravilhaCidade) + sizeof(tecDescoberta);
    f.seek(civ0 + jog * sizeof(Civ) + offsetof(Civ, povo));
    f.read(&c.povo, 1);
    if (c.povo >= N_POVOS) c.povo = 0;
    memcpy(c.nome, POVO[c.povo].lider, min(sizeof(c.nome) - 1, strlen(POVO[c.povo].lider))); c.nome[sizeof(c.nome) - 1] = 0;
  } else ok = false;
  f.close();
  return ok;
}

bool haJogoGravado() {
  Cabecalho c;
  for (int k = GRAV_AUTO; k <= GRAV_ANTIGA; ++k) if (leCabecalho(k, c)) return true;
  return false;
}

bool carregaJogo(int k) {
  if (!montaSD()) return false;
  char nf[20]; nomeFicheiro(nf, k);
  File f = SD.open(nf, FILE_READ);
  if (!f) return false;
  uint32_t m = 0;
  f.read((uint8_t *)&m, 4);
  Cabecalho c; memset(&c, 0, sizeof(c));
  Cabecalho3 c3 = { 80, 50, { 1, 1, 1, 1, 1, 1, 1 } };     // as versões anteriores eram sempre 80x50
  if (m == MAGICO2 || m == MAGICO3) f.read((uint8_t *)&c, sizeof(c));
  else if (m != MAGICO) { f.close(); return false; }
  if (m == MAGICO3) f.read((uint8_t *)&c3, sizeof(c3));
  if (c3.mapaW < 16 || c3.mapaW > MAPA_W_MAX || c3.mapaH < 16 || c3.mapaH > MAPA_H_MAX) { f.close(); return false; }
  poeTamanhoMapa(c3.mapaW, c3.mapaH);
  mundo = c3.mundo;
  leDados(f);
  f.close();
  strncpy(nomeJogador, c.nome, sizeof(nomeJogador) - 1); nomeJogador[sizeof(nomeJogador) - 1] = 0;
  verInimigos = c.livre & 1;
  ultimaGravacao = (k >= 0 && k < N_GRAVACOES) ? k : -1;
  rngEstado = millis() | 1;
  return true;
}

// texto de uma gravação: "Fernando (Portugueses)" e, à direita, o ano
void descreveGravacao(const Cabecalho & c, char * t, int nt, char * dir, int nd) {
  snprintf(t, nt, "%s (%s)", c.nome, POVO[c.povo % N_POVOS].nome);
  char ano[24]; textoAno(ano, c.turno);
  snprintf(dir, nd, "%s", ano);
}

// ---------------- jogo novo ----------------
uint8_t povoEscolhido = 0;

bool sitioInicial(int x, int y) {
  uint8_t t = terreno(x, y);
  if (!(t == T_PRADO || t == T_PLANICIE || t == T_RIO)) return false;
  if (temAldeia(x, y)) return false;
  if (tamanhoContinente(continente(x, y)) < 40) return false;
  int bom = 0;
  for (int r = 1; r < 21; ++r) { uint8_t q = terreno(x + RAIO_DX[r], y + RAIO_DY[r]); if (q == T_PRADO || q == T_PLANICIE || q == T_RIO || q == T_FLORESTA || q == T_COLINAS) ++bom; }
  return bom >= 10;
}

void novoJogo() {
  geraMundo(sementeEscolhida);          // o mesmo mundo que se viu no ecrã "Criar o mundo"
  memset(U, 0, sizeof(Unidade) * MAX_UNIDADES);
  memset(CID, 0, sizeof(Cidade) * MAX_CIDADES);
  memset(CIV, 0, sizeof(CIV));
  memset(tecDescoberta, 0, sizeof(tecDescoberta));
  for (int w = 0; w < N_MARAVILHAS; ++w) maravilhaCidade[w] = -1;
  turno = 0; fimDeJogo = 0; nMsg = 0; nPedeProducao = 0; proposta = PR_NADA; encontrosPendentes = 0; nTecsNovas = 0; nChegadas = 0; nCenas = 0;
  jogador = POVO[povoEscolhido].cor;
  // os outros povos: cores ao acaso, um povo de cada cor
  uint8_t usados = 1 << jogador;
  CIV[jogador].povo = povoEscolhido;
  for (int n = 1; n < nCivsJogo; ++n) {
    int cor;
    do { cor = 1 + aleat(7); } while (usados >> cor & 1);
    usados |= 1 << cor;
    int cand[N_POVOS], nc = 0;
    for (int p = 0; p < N_POVOS; ++p) if (POVO[p].cor == cor && p != povoEscolhido) cand[nc++] = p;
    CIV[cor].povo = cand[aleat(nc)];
  }
  // bárbaros
  CIV[0].viva = 1; CIV[0].povo = SEM; CIV[0].governo = G_DESPOTISMO; CIV[0].capital = SEM;
  // coloca cada povo
  int sx[N_CIVS], sy[N_CIVS], ns = 0;
  for (int c = 1; c < N_CIVS; ++c) {
    if (!(usados >> c & 1)) continue;
    Civ & C = CIV[c];
    C.viva = 1; C.humano = (c == jogador); C.governo = G_DESPOTISMO; C.taxa = 4; C.luxo = 0;
    C.capital = SEM; C.pesquisa = SEM_TEC; C.ouro = 0;
    int x = 0, y = 0;
    for (int dmin = 12; dmin >= 4; dmin -= 2) {
      bool achou = false;
      for (int t = 0; t < 3000 && !achou; ++t) {
        x = aleat(MAPA_W); y = 4 + aleat(MAPA_H - 8);
        if (!sitioInicial(x, y)) continue;
        bool longe = true;
        for (int k = 0; k < ns; ++k) if (distancia(x, y, sx[k], sy[k]) < dmin) longe = false;
        if (longe) achou = true;
      }
      if (achou) break;
    }
    sx[ns] = x; sy[ns] = y; ++ns;
    int colonos = C.humano ? (dificuldade <= 1 ? 2 : 1) : (dificuldade >= 3 ? 2 : 1);
    for (int k = 0; k < colonos; ++k) criaUnidade(UN_COLONOS, c, x, y, SEM);
    if (C.humano && dificuldade == 0) criaUnidade(UN_MILICIA, c, x, y, SEM);
    revela(c, x, y, 1);
    if (!C.humano) escolhePesquisaIA(c);
  }
  ajustaTaxas(jogador);
  int u0 = -1;
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].civ == jogador) { u0 = i; break; }
  if (u0 >= 0) centraEm(U[u0].x, U[u0].y);
  ativa = u0;
}

// ---------------- unidades do jogador ----------------
bool unidadeEsperaOrdens(int i) {
  const Unidade & u = U[i];
  return u.viva && u.civ == jogador && u.mov > 0 && u.estado == EU_NADA;
}

int ultimaAtiva = -1;
void proximaUnidade() {
  int ini = ativa >= 0 ? ativa : ultimaAtiva;
  for (int k = 1; k <= MAX_UNIDADES; ++k) {
    int i = (ini + k + MAX_UNIDADES) % MAX_UNIDADES;
    if (unidadeEsperaOrdens(i)) {
      ativa = i; ultimaAtiva = i;
      if (!naVista(U[i].x, U[i].y, 2)) centraEm(U[i].x, U[i].y);
      return;
    }
  }
  ativa = -1;
}

// unidades que chegaram junto de uma cidade inimiga (marcha com a tecla A) e esperam a ordem de ataque
void registaChegada(int i, int ci) {
  for (int k = 0; k < nChegadas; ++k) if (chegouUnid[k] == i) { chegouCid[k] = ci; return; }
  if (nChegadas < MAX_CHEGADAS) { chegouUnid[nChegadas] = i; chegouCid[nChegadas] = ci; ++nChegadas; }
}
void tiraChegada(int k) { --nChegadas; for (int j = k; j < nChegadas; ++j) { chegouUnid[j] = chegouUnid[j + 1]; chegouCid[j] = chegouCid[j + 1]; } }
// direção de (x, y) para a casa vizinha (tx, ty); -1 se não for vizinha
int direcaoPara(int x, int y, int tx, int ty) {
  for (int d = 0; d < 8; ++d) if (embrulhaX(x + DIR_DX[d]) == tx && y + DIR_DY[d] == ty) return d;
  return -1;
}

// segue o destino (ordem "ir para")
void segueDestino(int i) {
  Unidade & u = U[i];
  for (int s = 0; s < 12 && u.viva && u.mov > 0; ++s) {
    if (u.x == u.gx && u.y == u.gy) { u.estado = EU_NADA; u.gx = SEM; return; }
    // marcha contra uma cidade estrangeira: pára na casa ao lado e pergunta se ataca
    int ca = cidadeEm(u.gx, u.gy);
    if (ca >= 0 && CID[ca].civ != u.civ && direcaoPara(u.x, u.y, u.gx, u.gy) >= 0) {
      u.estado = EU_NADA; u.gx = SEM;
      if (u.civ == jogador) registaChegada(i, ca);
      return;
    }
    ResMov r = passoPara(i, u.gx, u.gy, false);
    if (r != MV_OK && r != MV_EMBARCOU) { u.estado = EU_NADA; u.gx = SEM; return; }
    u.estado = EU_DESTINO;
  }
  if (u.viva && u.x == u.gx && u.y == u.gy) { u.estado = EU_NADA; u.gx = SEM; }
  if (u.viva && u.gx != SEM) {
    int ca = cidadeEm(u.gx, u.gy);
    if (ca >= 0 && CID[ca].civ != u.civ && direcaoPara(u.x, u.y, u.gx, u.gy) >= 0) {
      u.estado = EU_NADA; u.gx = SEM;
      if (u.civ == jogador) registaChegada(i, ca);
    }
  }
}

// ---------------- mensagens ----------------
int msgAtual = 0;
Modo modoDepoisMsg = M_MAPA;
void continuaTurno();

void mostraMensagemAtual() {
  const Mensagem & m = filaMsg[msgAtual];
  if (m.x >= 0 && m.y >= 0 && viu(jogador, m.x, m.y) && !naVista(m.x, m.y, 1)) { centraEm(m.x, m.y); desenhaTudo(); }
  caixaTexto(nullptr, m.texto, "ESPAÇO");
}
bool mostraMensagens() {
  if (nMsg == 0) return false;
  msgAtual = 0;
  entraModo(M_MENSAGEM);
  mostraMensagemAtual();
  return true;
}
void atualizaMensagem() {
  if (tempo < 8) return;
  if (!carregouFogo() && !carregouVolta() && !premiu(VK_SPACE)) return;
  somMenu();
  if (++msgAtual < nMsg) { mostraMensagemAtual(); return; }
  nMsg = 0;
  continuaTurno();
}

// ---------------- pergunta sim/não ----------------
enum IdPergunta : uint8_t { Q_NADA, Q_GUERRA, Q_PAZ, Q_TRIBUTO, Q_SAIR, Q_REVOLUCAO, Q_NAVE };
IdPergunta idPergunta = Q_NADA;
char textoPergunta[220];
int perguntaSel = 0, perguntaDir = 0;
void desenhaPergunta() {
  int larg = 46, linhas = linhasTexto(larg, textoPergunta);
  int h = linhas * 9 + 28, w = larg * 8 + 16;
  int x0 = (MAPA_PX_W - w) / 2, y0 = (MAPA_PX_H - h) / 2;
  caixa(x0, y0, x0 + w, y0 + h, C_PAINEL, C_MOLDURA);
  textoQuebrado(x0 + 8, y0 + 6, larg, textoPergunta, C_BRANCO);
  const char * op[2] = { "Sim", "Não" };
  for (int i = 0; i < 2; ++i) {
    int bx = x0 + w / 2 - 70 + i * 90;
    if (i == perguntaSel) retangulo(bx - 4, y0 + h - 14, bx + 44, y0 + h - 4, C_MOLDURA);
    textoCentro(bx - 4, bx + 44, y0 + h - 13, op[i], i == perguntaSel ? C_AMARELO : C_BRANCO);
  }
}
void pergunta(IdPergunta id, const char * fmt, ...) {
  idPergunta = id;
  va_list ap; va_start(ap, fmt); vsnprintf(textoPergunta, sizeof(textoPergunta), fmt, ap); va_end(ap);
  perguntaSel = 0;
  entraModo(M_PERGUNTA);
  desenhaPergunta();
}

// ---------------- listas: produção, pesquisa, ordens, menus ----------------
int turnosPara(int falta, int porTurno) { if (porTurno <= 0) return 99; return (falta + porTurno - 1) / porTurno; }

// painel à direita da lista de produção: quem defende a cidade e os edifícios que já tem
void painelProducao() {
  const int ci = cidadeAtual;
  if (ci < 0) return;
  const Cidade & k = CID[ci];
  char b[64];
  const int x0 = 396, x1 = ECRA_W - 5, y0 = 1, y1 = 181;
  caixa(x0, y0, x1, y1, C_PAINEL, C_MOLDURA);
  snprintf(b, sizeof(b), "%s (%d)", k.nome, k.tam);
  textoCentro(x0, x1, y0 + 4, b, C_AMARELO);
  // unidades na cidade, em miniatura (até 8)
  int lista[16], n = unidadesEm(k.x, k.y, lista, 16);
  texto(x0 + 8, y0 + 16, "Na cidade:", C_CINZA);
  for (int i = 0; i < n && i < 8; ++i) desenhaUnidade(x0 + 12 + (i % 4) * 54, y0 + 27 + (i / 4) * 22, lista[i], 1);
  if (n == 0) texto(x0 + 12, y0 + 32, "nenhuma unidade!", C_VERMELHO);
  if (n > 8) { snprintf(b, sizeof(b), "+%d", n - 8); textoDireita(x1 - 6, y0 + 16, b, C_CINZA); }
  // edifícios (duas colunas)
  int ne = 0;
  for (int e = 0; e < N_EDIFICIOS; ++e) if (temEdif(k, e)) ++ne;
  snprintf(b, sizeof(b), "Edifícios (%d):", ne);
  texto(x0 + 8, y0 + 74, b, C_AMARELO);
  int j = 0;
  for (int e = 0; e < N_EDIFICIOS; ++e) if (temEdif(k, e)) {
    if (j < 20) {
      char nome[16]; strncpy(nome, EDIFICIO[e].nome, 14); nome[14] = 0;
      texto(x0 + 8 + (j / 10) * 116, y0 + 84 + (j % 10) * 9, nome, C_BRANCO);
    }
    ++j;
  }
  if (ne == 0) texto(x0 + 8, y0 + 84, "(nenhum)", C_CINZA_E);
  // em baixo: resumo das unidades, para se ver se a cidade está bem defendida
  caixa(8, 184, ECRA_W - 5, 197, C_PAINEL, C_MOLDURA);
  int def = 0, fort = 0;
  for (int i = 0; i < n; ++i) {
    const Unidade & u = U[lista[i]];
    if (eMilitar(u.tipo) && UNIDADE[u.tipo].dominio == D_TERRA) ++def;
    if (u.estado == EU_FORTIFICADA || u.estado == EU_FORTIFICANDO) ++fort;
  }
  char r[110];
  int p = snprintf(r, sizeof(r), "Unidades na cidade: %d (%d defensores, %d fortificadas)", n, def, fort);
  // os tipos, agrupados: "Falange x2, Colonos x1"
  bool visto[N_UNIDADES] = {};
  bool primeiro = true;
  for (int i = 0; i < n && p < (int)sizeof(r) - 20; ++i) {
    int t = U[lista[i]].tipo;
    if (visto[t]) continue;
    visto[t] = true;
    int c = 0; for (int q = 0; q < n; ++q) if (U[lista[q]].tipo == t) ++c;
    p += snprintf(r + p, sizeof(r) - p, "%s %s x%d", primeiro ? ":" : ",", UNIDADE[t].nome, c);
    primeiro = false;
  }
  if (strlen(r) > 77) { r[74] = r[75] = r[76] = '.'; r[77] = 0; }
  texto(14, 187, r, def == 0 ? C_VERMELHO : C_BRANCO);
  moldura();
}

void abreProducao(int ci, const char * titulo) {
  cidadeAtual = ci;
  modoAntesLista = modo;
  limpaLista(titulo, L_PRODUCAO);
  RendCidade r = calculaCidade(ci);
  char dir[16];
  int sel = 0;
  juntaItem(producaoAuto(CID[ci]) ? "Auto (ligado: o governador escolhe)" : "Auto (o governador escolhe)", PROD_AUTO, true, "A");
  if (producaoAuto(CID[ci])) sel = 0;
  for (int p = 0; p < N_UNIDADES; ++p) if (podeProduzir(ci, p)) {
    char t[44]; sprintf(t, "%s %d/%d/%d", UNIDADE[p].nome, UNIDADE[p].ataque, UNIDADE[p].defesa, UNIDADE[p].mov);
    sprintf(dir, "%d t.", turnosPara(UNIDADE[p].custo - CID[ci].escudos, r.sobraEscudos));
    if (p == CID[ci].prod && !producaoAuto(CID[ci])) sel = nItens;
    juntaItem(t, p, true, dir);
  }
  for (int e = 0; e < N_EDIFICIOS_TOTAL; ++e) if (podeProduzir(ci, PROD_EDIF + e)) {
    sprintf(dir, "%d t.", turnosPara(EDIFICIO[e].custo - CID[ci].escudos, r.sobraEscudos));
    if (PROD_EDIF + e == CID[ci].prod && !producaoAuto(CID[ci])) sel = nItens;
    juntaItem(EDIFICIO[e].nome, PROD_EDIF + e, true, dir);
  }
  for (int w = 0; w < N_MARAVILHAS; ++w) if (podeProduzir(ci, PROD_MARAV + w)) {
    char t[44]; snprintf(t, sizeof(t), "*%s", MARAVILHA[w].nome);
    sprintf(dir, "%d t.", turnosPara(MARAVILHA[w].custo - CID[ci].escudos, r.sobraEscudos));
    if (PROD_MARAV + w == CID[ci].prod && !producaoAuto(CID[ci])) sel = nItens;
    juntaItem(t, PROD_MARAV + w, true, dir);
  }
  itemSel = sel;
  listaX0 = 8; listaW = 384; listaLinhas = 15; listaY0 = 1;
  strcpy(ajudaLista, "* = maravilha   ESC: manter");
  extraLista = painelProducao; listaAltura = true;
  limpaEcra(C_PRETO);
  entraModo(M_LISTA);
  desenhaLista();
}

// por baixo da lista da pesquisa: o que é a tecnologia escolhida e o que permite
void painelPesquisa() {
  if (nItens == 0) return;
  int t = itens[itemSel].valor;
  const int x0 = 8, x1 = ECRA_W - 9, y0 = 134, y1 = 197;
  caixa(x0, y0, x1, y1, C_PAINEL, C_MOLDURA);
  texto(x0 + 8, y0 + 4, TEC[t].nome, C_AMARELO);
  // só a primeira frase do texto da enciclopédia (cabe em duas linhas)
  char frase[200]; const char * txt = TEC_TEXTO[t];
  int n = 0; while (txt[n] && n < (int)sizeof(frase) - 1) { frase[n] = txt[n]; if (txt[n] == '.' && (txt[n + 1] == ' ' || !txt[n + 1]) && n > 30) { ++n; break; } ++n; }
  frase[n] = 0;
  textoQuebrado(x0 + 8, y0 + 14, 76, frase, C_BRANCO);
  // o que passa a ser possível
  char b[200]; int p = snprintf(b, sizeof(b), "Permite:");
  bool algo = false;
  for (int u = 0; u < N_UNIDADES; ++u) if (UNIDADE[u].tec == t && p < 150) { p += snprintf(b + p, sizeof(b) - p, "%s %s", algo ? "," : "", UNIDADE[u].nome); algo = true; }
  for (int e = 0; e < N_EDIFICIOS_TOTAL; ++e) if (EDIFICIO[e].tec == t && p < 150) { p += snprintf(b + p, sizeof(b) - p, "%s %s", algo ? "," : "", EDIFICIO[e].nome); algo = true; }
  for (int w = 0; w < N_MARAVILHAS; ++w) if (MARAVILHA[w].tec == t && p < 150) { p += snprintf(b + p, sizeof(b) - p, "%s *%s", algo ? "," : "", MARAVILHA[w].nome); algo = true; }
  for (int g = G_DESPOTISMO + 1; g < N_GOVERNOS; ++g) if (GOVERNO[g].tec == t && p < 150) { p += snprintf(b + p, sizeof(b) - p, "%s governo: %s", algo ? "," : "", GOVERNO[g].nome); algo = true; }
  if (!algo) snprintf(b, sizeof(b), "Permite: abre caminho a novas descobertas.");
  if (strlen(b) > 76) { b[73] = b[74] = b[75] = '.'; b[76] = 0; }
  texto(x0 + 8, y0 + 43, b, C_VERDE);
  // o que se poderá estudar depois
  p = snprintf(b, sizeof(b), "Leva a:");
  int nl = 0;
  for (int q = 0; q < N_TECS; ++q) if ((TEC[q].pre1 == t || TEC[q].pre2 == t) && p < 150) { p += snprintf(b + p, sizeof(b) - p, "%s %s", nl ? "," : "", TEC[q].nome); ++nl; }
  if (nl) { if (strlen(b) > 76) { b[73] = b[74] = b[75] = '.'; b[76] = 0; } texto(x0 + 8, y0 + 33, b, C_CINZA); }
}

void abrePesquisa() {
  modoAntesLista = modo;
  limpaLista("Os sábios perguntam: o que devemos estudar?", L_PESQUISA);
  // lâmpadas por turno, para dizer quantos turnos leva
  int porTurno = 0;
  for (int i = 0; i < MAX_CIDADES; ++i) if (CID[i].viva && CID[i].civ == jogador) porTurno += calculaCidade(i).ciencia;
  int custo = custoTec(jogador), falta = max(0, custo - (int)CIV[jogador].lampadas);
  char dir[16];
  if (porTurno > 0) snprintf(dir, sizeof(dir), "%d turnos", (int)min(99, turnosPara(falta, porTurno)) % 100);
  else strcpy(dir, "");
  for (int t = 0; t < N_TECS; ++t) if (podePesquisar(jogador, t)) juntaItem(TEC[t].nome, t, true, dir);
  if (nItens == 0) { CIV[jogador].pesquisa = SEM_TEC; return; }
  char b[64]; snprintf(b, sizeof(b), "Custo: %d lâmpadas (%d por turno)", custo, porTurno);
  strcpy(ajudaLista, b);
  listaX0 = 120; listaW = 400; listaLinhas = 10; listaY0 = 4;
  extraLista = painelPesquisa;
  entraModo(M_LISTA);
  desenhaLista();
}

// ordens da unidade ativa (para quem joga com o joystick, ou ENTER)
enum Ordem { O_FUNDAR, O_JUNTAR, O_ESTRADA, O_FERROVIA, O_REGA, O_MINA, O_FORTALEZA, O_LIMPAR, O_FORTIFICAR,
             O_SENTINELA, O_IR, O_ATACAR, O_ESPERAR, O_NADA, O_CASA, O_DISPENSAR, O_VER_CIDADE, O_ACORDAR, O_FIM_TURNO, O_VER, O_MENU };
void abreOrdens() {
  modoAntesLista = M_MAPA;
  limpaLista("Ordens", L_ORDENS);
  if (ativa >= 0 && U[ativa].viva) {
    const Unidade & u = U[ativa];
    const InfoUnidade & I = UNIDADE[u.tipo];
    int ci = cidadeEm(u.x, u.y);
    if (I.flags & U_COLONO) {
      if (ci < 0) juntaItem("Fundar cidade", O_FUNDAR, !motivoFundar(jogador, u.x, u.y), "B");
      else if (CID[ci].civ == jogador) juntaItem("Juntar à cidade", O_JUNTAR, CID[ci].tam < 8, "B");
      if (!(melh(u.x, u.y) & M_ESTRADA)) juntaItem("Estrada", O_ESTRADA, !motivoTrabalho(jogador, u.x, u.y, TB_ESTRADA), "R");
      else juntaItem("Caminho de ferro", O_FERROVIA, !motivoTrabalho(jogador, u.x, u.y, TB_FERROVIA), "R");
      juntaItem("Regar", O_REGA, !motivoTrabalho(jogador, u.x, u.y, TB_REGA), "I");
      juntaItem("Mina", O_MINA, !motivoTrabalho(jogador, u.x, u.y, TB_MINA), "M");
      juntaItem("Fortaleza", O_FORTALEZA, !motivoTrabalho(jogador, u.x, u.y, TB_FORTALEZA), "F");
      if (melh(u.x, u.y) & M_POLUICAO) juntaItem("Limpar poluição", O_LIMPAR, true, "P");
    } else if (I.dominio == D_TERRA) juntaItem("Fortificar", O_FORTIFICAR, true, "F");
    juntaItem("Sentinela", O_SENTINELA, true, "S");
    juntaItem("Ir para...", O_IR, true, "G");
    if (podeMarchar(ativa)) juntaItem("Atacar uma cidade...", O_ATACAR, true, "A");
    juntaItem("Esperar", O_ESPERAR, true, "W");
    juntaItem("Sem ordens", O_NADA, true, "ESPAÇO");
    if (ci >= 0 && CID[ci].civ == jogador) {
      juntaItem("Mudar a casa para aqui", O_CASA, u.casa != ci, "H");
      juntaItem("Ver a cidade", O_VER_CIDADE, true, "C");
    }
    int lista[16], n = unidadesEm(u.x, u.y, lista, 16);
    if (n > 1) juntaItem(eOceano(u.x, u.y) && UNIDADE[u.tipo].carga > 0 ? "Desembarcar (acordar os passageiros)" : "Acordar as outras",
                         O_ACORDAR, true, "U");
    juntaItem("Dispensar a unidade", O_DISPENSAR, true, "D");
  }
  juntaItem("Modo ver (cursor)", O_VER, true, "V");
  juntaItem("Fim do turno", O_FIM_TURNO, true, "ENTER");
  juntaItem("Menu do jogo", O_MENU, true, "ESC");
  listaX0 = 120; listaW = 300; listaLinhas = 18;
  entraModo(M_LISTA);
  desenhaLista();
}

enum Menu { MN_TAXAS, MN_REVOLUCAO, MN_DIPLOMACIA, MN_CIDADES, MN_CIENCIA, MN_POVOS, MN_MARAVILHAS, MN_PONTOS,
            MN_NAVE, MN_GRAVAR, MN_MUSICA, MN_INIMIGOS, MN_SAIR, MN_VOLTAR, MN_DORMIR };
bool podeLancarNave(int civ) {
  const Civ & C = CIV[civ];
  return C.naveChega == 0 && C.nave[0] >= 4 && C.nave[1] >= 2 && C.nave[2] >= 1 && C.capital != SEM;
}
void abreMenu() {
  modoAntesLista = M_MAPA;
  limpaLista("Menu do jogo", L_MENU);
  juntaItem("As nossas cidades", MN_CIDADES, true, "F1");
  juntaItem("Unidades a dormir (sentinela, fortificadas)", MN_DORMIR, true, "F2");
  juntaItem("Impostos, luxo e ciência", MN_TAXAS, true, "T");
  juntaItem("Revolução (mudar de governo)", MN_REVOLUCAO, CIV[jogador].anarquia == 0, "X");
  juntaItem("Diplomacia", MN_DIPLOMACIA, true, "Y");
  juntaItem("Conselho de ciência", MN_CIENCIA, true, "F6");
  juntaItem("Os povos do mundo", MN_POVOS, true, "F3");
  juntaItem("Maravilhas do mundo", MN_MARAVILHAS, true, "F7");
  juntaItem("Pontuação", MN_PONTOS, true, "F9");
  if (podeLancarNave(jogador)) juntaItem("Lançar a nave espacial!", MN_NAVE, true);
  juntaItem("Gravar o jogo", MN_GRAVAR, true, "F10");
  juntaItem(musicaPermitida ? "Música: ligada" : "Música: desligada", MN_MUSICA, true);
  juntaItem(verInimigos ? "Ver movimentos dos outros: sim" : "Ver movimentos dos outros: não", MN_INIMIGOS, true);
  juntaItem("Sair para o título", MN_SAIR, true);
  juntaItem("Voltar ao jogo", MN_VOLTAR, true, "ESC");
  listaX0 = 120; listaW = 300; listaLinhas = 15;
  entraModo(M_LISTA);
  desenhaLista();
}

// ---------------- relatórios ----------------
int paginaRelatorio = 0, tipoRelatorio = 0;
void desenhaRelatorio() {
  limpaEcra(C_PAINEL);
  char b[120], ano[24];
  textoAno(ano, turno);
  switch (tipoRelatorio) {
    case MN_CIENCIA: {
      textoCentro(0, ECRA_W - 1, 2, "Conselho de Ciência", C_AMARELO);
      const Civ & C = CIV[jogador];
      sprintf(b, "A estudar: %s   %d de %d lâmpadas", C.pesquisa == SEM_TEC ? "nada" : TEC[C.pesquisa].nome, C.lampadas, custoTec(jogador));
      texto(8, 14, b, C_CEU);
      texto(8, 26, "Conhecemos:", C_BRANCO);
      int n = 0;
      for (int t = 0; t < N_TECS; ++t) if (temTec(jogador, t)) {
        int col = n / 15, lin = n % 15;
        if (col < 4) texto(8 + col * 158, 36 + lin * 9, TEC[t].nome, C_CINZA);
        ++n;
      }
      sprintf(b, "%d de %d tecnologias", n, N_TECS);
      texto(8, 178, b, C_OURO);
      break;
    }
    case MN_POVOS: {
      textoCentro(0, ECRA_W - 1, 2, "Os povos do mundo", C_AMARELO);
      texto(8, 14, "Povo            Líder                Governo      Cid. Tec.  Relação", C_CINZA);
      int y = 26;
      for (int c = 1; c < N_CIVS; ++c) {
        if (!CIV[c].viva) continue;
        bool conhece = c == jogador || (CIV[jogador].contacto >> c & 1);
        if (!conhece) continue;
        RGB888 cor = corCiv(corDe(c));
        texto(8, y, nomePovo(c), cor);
        texto(136, y, nomeLider(c), C_BRANCO);
        texto(304, y, GOVERNO[CIV[c].governo].nome, C_BRANCO);
        sprintf(b, "%3d %4d", contaCidades(c), CIV[c].nTecs);
        texto(408, y, b, C_BRANCO);
        texto(480, y, c == jogador ? "(nós)" : (CIV[jogador].guerra >> c & 1) ? "GUERRA" : "paz", c == jogador ? C_CINZA : (CIV[jogador].guerra >> c & 1) ? C_VERMELHO : C_VERDE);
        y += 12;
      }
      int nao = 0;
      for (int c = 1; c < N_CIVS; ++c) if (CIV[c].viva && c != jogador && !(CIV[jogador].contacto >> c & 1)) ++nao;
      if (nao) { sprintf(b, "Há ainda %d povo(s) que não conhecemos.", nao); texto(8, y + 6, b, C_CINZA); }
      break;
    }
    case MN_MARAVILHAS: {
      textoCentro(0, ECRA_W - 1, 2, "Maravilhas do mundo", C_AMARELO);
      for (int w = 0; w < N_MARAVILHAS; ++w) {
        int y = 12 + w * 8;
        int ci = maravilhaCidade[w];
        texto(8, y, MARAVILHA[w].nome, ci >= 0 ? C_OURO : C_CINZA_E);
        if (ci >= 0 && CID[ci].viva) {
          sprintf(b, "%s (%s)", CID[ci].nome, nomePovo(CID[ci].civ));
          texto(264, y, b, corCiv(corDe(CID[ci].civ)));
        } else if (ci == -2) texto(264, y, "destruída", C_CINZA_E);
        else texto(264, y, MARAVILHA[w].efeito, C_CINZA_E);
      }
      break;
    }
    case MN_PONTOS: {
      textoCentro(0, ECRA_W - 1, 2, "Pontuação", C_AMARELO);
      int y = 20;
      for (int c = 1; c < N_CIVS; ++c) {
        if (!CIV[c].viva) continue;
        if (c != jogador && !(CIV[jogador].contacto >> c & 1)) continue;
        sprintf(b, "%-14s %5d pontos", nomePovo(c), calculaPontos(c));
        texto(160, y, b, corCiv(corDe(c)));
        y += 12;
      }
      texto(40, 150, "Cidadãos felizes 2, contentes 1; cada maravilha 20; cada tecnologia 2.", C_CINZA);
      break;
    }
  }
  // ano e ajuda em cima, para não tapar as listas compridas
  texto(8, 2, ano, C_CINZA);
  textoDireita(ECRA_W - 8, 2, "ESC: voltar", C_CINZA);
  moldura();
}
void abreRelatorio(int tipo) { tipoRelatorio = tipo; entraModo(M_RELATORIO); desenhaRelatorio(); }

void abreListaCidades() {
  modoAntesLista = M_MAPA;
  limpaLista("As nossas cidades", L_CIDADES);
  for (int i = 0; i < MAX_CIDADES; ++i) {
    const Cidade & k = CID[i];
    if (!k.viva || k.civ != jogador) continue;
    RendCidade r = calculaCidade(i);
    char t[44], d[16];
    snprintf(t, sizeof(t), "%-12s %2d %s", k.nome, k.tam, r.desordem ? "DESORDEM" : nomeProd(k.prod));
    sprintf(d, "%+d %d %d", r.sobraComida, r.sobraEscudos, r.comercio);
    juntaItem(t, i, true, d);
  }
  strcpy(ajudaLista, "comida/escudos/comércio   ESPAÇO: abrir");
  listaX0 = 80; listaW = 420; listaLinhas = 16;
  entraModo(M_LISTA);
  desenhaLista();
}

void abreGoverno() {
  modoAntesLista = M_MAPA;
  limpaLista("Revolução! Que governo queremos?", L_GOVERNO);
  for (int g = G_DESPOTISMO; g < N_GOVERNOS; ++g)
    if (temTec(jogador, GOVERNO[g].tec)) juntaItem(GOVERNO[g].nome, g, g != CIV[jogador].governo);
  strcpy(ajudaLista, temMaravilha(jogador, W_PIRAMIDES) ? "As Pirâmides evitam a anarquia." : "Haverá 2 turnos de anarquia.");
  listaX0 = 160; listaW = 260; listaLinhas = 8;
  entraModo(M_LISTA);
  desenhaLista();
}

void abreDiplomacia() {
  modoAntesLista = M_MAPA;
  limpaLista("Diplomacia: com quem falamos?", L_DIPLOMACIA);
  for (int c = 1; c < N_CIVS; ++c) {
    if (c == jogador || !CIV[c].viva || !(CIV[jogador].contacto >> c & 1)) continue;
    char t[44]; snprintf(t, sizeof(t), "%s (%s)", nomePovo(c), POVO[CIV[c].povo].lider);
    juntaItem(t, c, true, (CIV[jogador].guerra >> c & 1) ? "guerra" : "paz");
  }
  if (nItens == 0) { mensagem("Ainda não conhecemos nenhum outro povo."); mostraMensagens(); return; }
  listaX0 = 100; listaW = 380; listaLinhas = 8;
  entraModo(M_LISTA);
  desenhaLista();
}

// ---------------- taxas ----------------
int taxaSel = 0;
void desenhaTaxas() {
  const Civ & C = CIV[jogador];
  int x0 = 120, x1 = 420, y0 = 50;
  caixa(x0, y0, x1, y0 + 72, C_PAINEL, C_MOLDURA);
  textoCentro(x0, x1, y0 + 5, "Impostos, luxo e ciência", C_AMARELO);
  char b[48];
  int vals[3] = { C.taxa, C.luxo, 10 - C.taxa - C.luxo };
  const char * nomes[3] = { "Impostos", "Luxo", "Ciência" };
  for (int i = 0; i < 3; ++i) {
    int y = y0 + 20 + i * 12;
    if (i == taxaSel) retangulo(x0 + 6, y - 1, x1 - 6, y + 8, C_MOLDURA);
    sprintf(b, "%-10s %3d%%", nomes[i], vals[i] * 10);
    texto(x0 + 12, y, b, i == taxaSel ? C_AMARELO : C_BRANCO);
    for (int k = 0; k < 10; ++k) retangulo(x0 + 140 + k * 14, y, x0 + 151 + k * 14, y + 6, k < vals[i] ? C_OURO : C_CINZA_E);
  }
  sprintf(b, "Máximo no %s: %d%%", GOVERNO[C.governo].nome, GOVERNO[C.governo].taxaMax * 10);
  texto(x0 + 12, y0 + 58, b, C_CINZA);
}
void atualizaTaxas() {
  Civ & C = CIV[jogador];
  if (menuCima()) { taxaSel = (taxaSel + 2) % 3; somMenu(); desenhaTaxas(); }
  if (menuBaixo()) { taxaSel = (taxaSel + 1) % 3; somMenu(); desenhaTaxas(); }
  int d = menuDir() ? 1 : menuEsq() ? -1 : 0;
  if (d) {
    int mx = GOVERNO[C.governo].taxaMax;
    int v[3] = { C.taxa, C.luxo, 10 - C.taxa - C.luxo };
    int nv = v[taxaSel] + d;
    if (nv >= 0 && nv <= mx) {
      // tira (ou dá) à seguinte que puder
      for (int k = 1; k <= 2; ++k) {
        int o = (taxaSel + k) % 3;
        if (v[o] - d >= 0 && v[o] - d <= mx) { v[taxaSel] = nv; v[o] -= d; break; }
      }
      C.taxa = v[0]; C.luxo = v[1];
      somMenu(); desenhaTaxas();
    } else somErro();
  }
  if (carregouVolta() || carregouFogo()) { entraModo(M_MAPA); desenhaTudo(); }
}

// ---------------- ecrã da cidade ----------------
int cidCurX = 2, cidCurY = 2;
const int CID_MX = 8, CID_MY = 12;          // canto do mapa 5x5 da cidade (240x100)

int raioDe(int dx, int dy) {
  for (int r = 0; r < 21; ++r) if (RAIO_DX[r] == dx && RAIO_DY[r] == dy) return r;
  return -1;
}

int precoCompra(const Cidade & k) {
  int falta = custoProd(k.prod) - k.escudos;
  if (falta <= 0) return 0;
  int p = 2 * falta + falta * falta / 20;
  if (k.escudos == 0) p *= 2;
  if (k.prod >= PROD_MARAV) p *= 2;
  return p;
}

// ícone de um cidadão (10x13): cabelo, cara, corpo e pernas
void desenhaCidadao(int x, int y, RGB888 roupa, RGB888 cara, char marca) {
  canvas.setBrushColor(C_PRETO);
  canvas.fillRectangle(x, y, x + 9, y + 12);
  canvas.setBrushColor(RGB888(85, 0, 0));
  canvas.fillRectangle(x + 3, y + 1, x + 6, y + 1);         // cabelo
  canvas.setBrushColor(cara);
  canvas.fillRectangle(x + 3, y + 2, x + 6, y + 4);         // cara
  canvas.setBrushColor(roupa);
  canvas.fillRectangle(x + 1, y + 5, x + 8, y + 8);         // corpo e braços
  canvas.fillRectangle(x + 2, y + 9, x + 3, y + 11);        // pernas
  canvas.fillRectangle(x + 6, y + 9, x + 7, y + 11);
  if (marca) {
    char t[2] = { marca, 0 };
    canvas.setPenColor(C_PRETO);
    canvas.drawText(&fonte, x + 1, y + 3, t);
  }
}

// ícones dos recursos (8x5, com contorno): comida (espiga), escudos e comércio (seta)
const int ICO_W = 9, ICO_H = 6;
const char * const ICONE_REC[3][ICO_H] = {
  { "..KKKKK..", ".KYYOYYK.", "KYOYYYOYK", "KOYYOYYOK", ".KGGGGGK.", "..KKKKK.." },   // comida: espiga
  { "KKKKKKKK.", "KWWggggK.", "KWggggkK.", ".KgggkK..", "..KgkK...", "...KK...." },   // escudos
  { ".....K...", "KKKKKYK..", "KYYYYYYK.", "KOOOOOOK.", "KKKKKOK..", ".....K..." },   // comércio: seta
};
RGB888 corIcone(char c) {
  switch (c) {
    case 'K': return C_PRETO;
    case 'Y': return RGB888(255, 255, 85);
    case 'O': return RGB888(255, 170, 0);
    case 'G': return RGB888(0, 170, 0);
    case 'W': return C_BRANCO;
    case 'g': return RGB888(170, 170, 170);
    case 'k': return RGB888(85, 85, 85);
  }
  return C_PRETO;
}
void desenhaIconeRec(int x, int y, int tipo) {
  for (int r = 0; r < ICO_H; ++r) {
    const char * l = ICONE_REC[tipo][r];
    for (int c = 0; l[c]; ) {
      if (l[c] == '.') { ++c; continue; }
      int c0 = c; char ch = l[c];
      while (l[c] == ch) ++c;                       // um traço de cada vez
      canvas.setBrushColor(corIcone(ch));
      canvas.fillRectangle(x + c0, y + r, x + c - 1, y + r);
    }
  }
}
// uma fila de n ícones a começar em x (aperta-os se não couberem em larg)
void filaIcones(int x, int y, int n, int tipo, int larg) {
  if (n <= 0) return;
  int passo = n <= 1 ? 8 : min(8, (larg - ICO_W) / (n - 1));
  for (int i = 0; i < n; ++i) desenhaIconeRec(x + i * passo, y, tipo);
}
// rendimento de uma casa no mapa da cidade, como no original: uma fila de espigas, uma de escudos e uma de setas
void desenhaRendimento(int sx, int sy, Rend v) {
  int fila = 0;
  const int val[3] = { v.comida, v.escudos, v.comercio };
  for (int i = 0; i < 3; ++i) {
    if (val[i] <= 0) continue;
    filaIcones(sx + 2, sy + 1 + fila * ICO_H, val[i], i, TILE_W - 4);
    ++fila;
  }
}

void desenhaCidade() {
  Cidade & k = CID[cidadeAtual];
  RendCidade r = calculaCidade(cidadeAtual);
  limpaEcra(C_PAINEL);
  char b[120], ano[24];
  textoAno(ano, turno);
  sprintf(b, "%s (%d)   %s   %s   Ouro %ld", k.nome, k.tam, nomePovo(k.civ), ano, (long)CIV[k.civ].ouro);
  textoCentro(0, ECRA_W - 1, 2, b, C_AMARELO);
  // ---- mapa 5x5 das casas da cidade ----
  for (int dy = -2; dy <= 2; ++dy)
    for (int dx = -2; dx <= 2; ++dx) {
      int sx = CID_MX + (dx + 2) * TILE_W, sy = CID_MY + (dy + 2) * TILE_H;
      int q = raioDe(dx, dy);
      int x = k.x + dx, y = k.y + dy;
      if (q < 0) { retangulo(sx, sy, sx + TILE_W - 1, sy + TILE_H - 1, C_PRETO); continue; }
      desenhaCasa(sx, sy, x, y, false);
      bool trab = q == 0 || (k.trab >> q & 1);
      if (trab) {
        Rend v = rendimentoCasa(k.civ, x, y, q == 0);
        if (eOceano(x, y) && temEdif(k, E_PORTO)) ++v.comida;
        desenhaRendimento(sx, sy, v);
      } else if (!casaDisponivel(cidadeAtual, q)) {
        canvas.setPenColor(C_VERMELHO);
        canvas.drawLine(sx + 3, sy + 2, sx + TILE_W - 4, sy + TILE_H - 3);
        canvas.drawLine(sx + 3, sy + TILE_H - 3, sx + TILE_W - 4, sy + 2);
      }
    }
  {   // cursor
    int sx = CID_MX + cidCurX * TILE_W, sy = CID_MY + cidCurY * TILE_H;
    canvas.setPenColor(C_BRANCO);
    canvas.drawRectangle(sx, sy, sx + TILE_W - 1, sy + TILE_H - 1);
    canvas.drawRectangle(sx + 1, sy + 1, sx + TILE_W - 2, sy + TILE_H - 2);
  }
  // ---- cidadãos ----
  // (os cidadãos ficam por baixo do mapa; se forem muitos, encolhe-se o espaço entre eles)
  int nCid = r.felizes + r.contentes + r.descontentes + nEspecialistas(k);
  int passo = nCid > 0 ? min(12, 234 / nCid) : 12;
  int x = 8, y = 115;
  for (int i = 0; i < r.felizes; ++i)      { desenhaCidadao(x, y, C_AMARELO, RGB888(255, 170, 85), 0); x += passo; }
  for (int i = 0; i < r.contentes; ++i)    { desenhaCidadao(x, y, C_BRANCO, RGB888(255, 170, 85), 0); x += passo; }
  for (int i = 0; i < r.descontentes; ++i) { desenhaCidadao(x, y, C_VERMELHO, RGB888(170, 85, 85), 0); x += passo; }
  for (int i = 0; i < k.especialistas[0]; ++i) { desenhaCidadao(x, y, C_CEU, RGB888(255, 170, 85), 'A'); x += passo; }
  for (int i = 0; i < k.especialistas[1]; ++i) { desenhaCidadao(x, y, C_OURO, RGB888(255, 170, 85), '$'); x += passo; }
  for (int i = 0; i < k.especialistas[2]; ++i) { desenhaCidadao(x, y, C_VERDE, RGB888(255, 170, 85), 'C'); x += passo; }
  // ---- recursos (linhas curtas: cabem por baixo do mapa, até x = 248) ----
  sprintf(b, "Comida   %3d (-%d) %+d", r.comida, r.consumo, r.sobraComida);
  texto(8, 131, b, C_VERDE);
  sprintf(b, "Escudos  %3d (-%d) %+d", r.escudos, r.apoio, r.sobraEscudos);
  texto(8, 140, b, C_OURO);
  sprintf(b, "Comércio %3d (-%d)", r.comercio, r.corrupcao);
  texto(8, 149, b, C_AMARELO);
  // os mesmos totais em ícones, como no original
  filaIcones(172, 132, r.comida, 0, 78);
  filaIcones(172, 141, r.escudos, 1, 78);
  filaIcones(172, 150, r.comercio, 2, 78);
  sprintf(b, "Ouro %+d Luxo %d Ciên. %d", r.ouro, r.luxo, r.ciencia);
  texto(8, 158, b, C_CINZA);
  // celeiro
  int caixa = (k.tam + 1) * 10;
  texto(8, 168, "Celeiro", C_VERDE);
  canvas.setPenColor(C_CINZA_E);
  canvas.drawRectangle(68, 168, 169, 174);
  int w = constrain(k.comida * 100 / caixa, 0, 100);
  if (w > 0) retangulo(69, 169, 68 + w, 173, C_VERDE_E);
  sprintf(b, "%d/%d", k.comida, caixa);
  texto(176, 168, b, C_CINZA);
  // felicidade (à direita, por baixo das unidades)
  if (r.desordem) {
    texto(256, 124, "DESORDEM CIVIL!", C_VERMELHO);
    texto(256, 133, "Põe artistas ou", C_VERMELHO);
    texto(256, 142, "aumenta o luxo.", C_VERMELHO);
  } else {
    sprintf(b, "Felizes %d", r.felizes);           texto(256, 124, b, C_AMARELO);
    sprintf(b, "Contentes %d", r.contentes);       texto(256, 133, b, C_BRANCO);
    sprintf(b, "Descontentes %d", r.descontentes); texto(256, 142, b, C_VERMELHO);
  }
  // ---- produção ----
  const int px = 256;
  texto(px, 12, "A fazer:", C_CINZA);
  texto(px, 21, nomeProd(k.prod), C_BRANCO);
  int custo = custoProd(k.prod);
  canvas.setPenColor(C_CINZA_E);
  canvas.drawRectangle(px, 31, px + 181, 37);
  int wp = constrain(k.escudos * 180 / max(1, custo), 0, 180);
  if (wp > 0) retangulo(px + 1, 32, px + wp, 36, C_OURO);
  sprintf(b, "%d/%d   %d turnos", k.escudos, custo, turnosPara(custo - k.escudos, r.sobraEscudos));
  texto(px, 41, b, C_CINZA);
  int preco = precoCompra(k);
  sprintf(b, "Comprar: %d$", preco);
  texto(px, 50, b, k.comprou ? C_CINZA_E : (CIV[k.civ].ouro >= preco ? C_OURO : C_CINZA_E));
  // ---- unidades na cidade e sustentadas ----
  int lista[16], n = unidadesEm(k.x, k.y, lista, 16), sust = 0;
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].casa == cidadeAtual) ++sust;
  sprintf(b, "Aqui: %d  Sustenta: %d", n, sust);
  texto(px, 64, b, C_CINZA);
  for (int i = 0; i < n && i < 8; ++i) {
    // miniatura da unidade (o desenho de 48x20 numa grelha de 4x2)
    int ux = px + (i % 4) * 46, uy = 74 + (i / 4) * 22;
    desenhaUnidade(ux, uy, lista[i], 1);
  }
  // ---- edifícios ----
  const int ex = 448;
  texto(ex, 12, "Edifícios", C_AMARELO);
  int k2 = 0;
  auto linha = [&](const char * nome, RGB888 cor) {
    if (k2 < 18) texto(ex, 22 + k2 * 9, nome, cor);
    else if (k2 == 18) texto(ex, 22 + k2 * 9, "...", C_CINZA);
    ++k2;
  };
  for (int e = 0; e < N_EDIFICIOS; ++e) if (temEdif(k, e)) linha(EDIFICIO[e].nome, C_BRANCO);
  for (int w2 = 0; w2 < N_MARAVILHAS; ++w2) if (maravilhaCidade[w2] == cidadeAtual) linha(MARAVILHA[w2].nome, C_OURO);
  // ---- ajuda ----
  texto(8, 181, "Setas: casa  ESPAÇO: trabalhar/largar", C_CINZA);
  texto(8, 190, "P produção  U unidades  C comprar  V vender  E especial.  ESC sair", C_CINZA);
  moldura();
}

// ao entrar numa cidade abre-se logo a escolha da produção por cima do ecrã da cidade
// (ESC mantém o que está a ser feito e fica-se no ecrã da cidade)
void abreCidade(int ci, bool comProducao = true) {
  cidadeAtual = ci;
  cidCurX = 2; cidCurY = 2;
  arrumaCidade(ci);
  entraModo(M_CIDADE);
  desenhaCidade();
  if (comProducao) abreProducao(ci, CID[ci].nome);
}

// ---- unidades na cidade: acordar as que estão de sentinela ou fortificadas ----
const char * nomeEstado(const Unidade & u) {
  switch (u.estado) {
    case EU_FORTIFICANDO: case EU_FORTIFICADA: return "fortificada";
    case EU_SENTINELA: return "sentinela";
    case EU_TRABALHO: return "a trabalhar";
    case EU_DESTINO: return "em viagem";
    case EU_A_BORDO: return "a bordo";
  }
  return u.mov > 0 ? "pronta" : "pronta (próx. turno)";
}
const int ACORDAR_TODAS = 1000;
void abreUnidadesCidade(int sel = -1) {
  const Cidade & k = CID[cidadeAtual];
  modoAntesLista = M_CIDADE;
  char t[48]; snprintf(t, sizeof(t), "Unidades em %s", k.nome);
  limpaLista(t, L_UNID_CIDADE);
  int lista[16], n = unidadesEm(k.x, k.y, lista, 16), dormem = 0;
  for (int j = 0; j < n; ++j) if (U[lista[j]].civ == jogador && U[lista[j]].estado != EU_NADA) ++dormem;
  juntaItem("Acordar todas", ACORDAR_TODAS, dormem > 0, "T");
  for (int j = 0; j < n; ++j) {
    const Unidade & u = U[lista[j]];
    if (u.civ != jogador) continue;
    char nome[44];
    snprintf(nome, sizeof(nome), "%s%s", UNIDADE[u.tipo].nome, u.vet ? " (vet.)" : "");
    if (lista[j] == sel) itemSel = nItens;
    juntaItem(nome, lista[j], true, nomeEstado(u));
  }
  strcpy(ajudaLista, "ESPAÇO: acordar e mover   ESC: voltar");
  listaX0 = 140; listaW = 360; listaLinhas = 12;
  entraModo(M_LISTA);
  desenhaLista();
}
void acordaUnidade(int i) {
  Unidade & u = U[i];
  if (u.estado == EU_TRABALHO) { u.trabalho = TB_NADA; u.prog = 0; }
  u.estado = EU_NADA;
  u.gx = SEM;
}

void compraNaCidade() {
  Cidade & k = CID[cidadeAtual];
  int preco = precoCompra(k);
  if (k.comprou || preco <= 0) { somErro(); return; }
  if (CIV[k.civ].ouro < preco) { somErro(); return; }
  CIV[k.civ].ouro -= preco;
  k.escudos = custoProd(k.prod);
  k.comprou = 1;
  somOk();
  desenhaCidade();
}

void abreVender() {
  Cidade & k = CID[cidadeAtual];
  modoAntesLista = M_CIDADE;
  limpaLista("Vender um edifício (uma vez por turno)", L_VENDER);
  for (int e = 1; e < N_EDIFICIOS; ++e) if (temEdif(k, e)) {
    char d[16]; sprintf(d, "%d$", EDIFICIO[e].custo);
    juntaItem(EDIFICIO[e].nome, e, !k.comprou, d);
  }
  if (nItens == 0) { somErro(); return; }
  listaX0 = 160; listaW = 300; listaLinhas = 14;
  entraModo(M_LISTA);
  desenhaLista();
}

void opcoesCidade() {
  modoAntesLista = M_CIDADE;
  limpaLista(CID[cidadeAtual].nome, L_OPCOES_CIDADE);
  juntaItem("Mudar a produção", 0, true, "P");
  char d[16]; sprintf(d, "%d$", precoCompra(CID[cidadeAtual]));
  juntaItem("Comprar", 1, !CID[cidadeAtual].comprou && precoCompra(CID[cidadeAtual]) > 0, d);
  juntaItem("Vender um edifício", 2, true, "V");
  juntaItem("Mudar especialista", 3, nEspecialistas(CID[cidadeAtual]) > 0, "E");
  juntaItem("Unidades na cidade", 5, true, "U");
  juntaItem("Sair", 4, true, "ESC");
  listaX0 = 200; listaW = 240; listaLinhas = 7;
  entraModo(M_LISTA);
  desenhaLista();
}

void atualizaCidade() {
  Cidade & k = CID[cidadeAtual];
  bool mudou = false;
  if (menuCima() && cidCurY > 0) { --cidCurY; mudou = true; }
  if (menuBaixo() && cidCurY < 4) { ++cidCurY; mudou = true; }
  if (menuEsq() && cidCurX > 0) { --cidCurX; mudou = true; }
  if (menuDir() && cidCurX < 4) { ++cidCurX; mudou = true; }
  if (mudou) { somMenu(); desenhaCidade(); }
  if (carregouVolta()) { entraModo(M_MAPA); desenhaTudo(); return; }
  if (premiuLetra('P')) { abreProducao(cidadeAtual, k.nome); return; }
  if (premiuLetra('C')) { compraNaCidade(); return; }
  if (premiuLetra('V')) { abreVender(); return; }
  if (premiuLetra('U')) { abreUnidadesCidade(); return; }
  if (premiuLetra('E')) {
    // passa um especialista para o tipo seguinte: artista -> cobrador -> cientista
    for (int s = 0; s < 3; ++s) if (k.especialistas[s]) { --k.especialistas[s]; ++k.especialistas[(s + 1) % 3]; break; }
    somMenu(); desenhaCidade(); return;
  }
  if (carregouFogo()) {
    int r = raioDe(cidCurX - 2, cidCurY - 2);
    if (r == 0) { opcoesCidade(); return; }
    if (r < 0) { somErro(); return; }
    if (k.trab >> r & 1) {
      k.trab &= ~(1UL << r);
      ++k.especialistas[0];
    } else {
      if (!casaDisponivel(cidadeAtual, r)) { somErro(); return; }
      if (nEspecialistas(k) > 0) {
        for (int s = 0; s < 3; ++s) if (k.especialistas[s]) { --k.especialistas[s]; break; }
      } else {
        // troca pela pior casa trabalhada
        int pior = -1, pv = 9999;
        for (int q = 1; q < 21; ++q) if (k.trab >> q & 1) { int v = valorCasa(cidadeAtual, q, false); if (v < pv) { pv = v; pior = q; } }
        if (pior < 0) { somErro(); return; }
        k.trab &= ~(1UL << pior);
      }
      k.trab |= 1UL << r;
    }
    somOk();
    desenhaCidade();
  }
}

// ---------------- diplomatas e caravanas ----------------
int unidadeAcao = -1;
void abreDiplomata(int i, int x, int y) {
  unidadeAcao = i; alvoX = x; alvoY = y;
  int ci = cidadeEm(x, y);
  if (ci < 0) return;
  modoAntesLista = M_MAPA;
  char t[60]; snprintf(t, sizeof(t), "Diplomata em %s", CID[ci].nome);
  limpaLista(t, L_DIPLOMATA);
  int civ = CID[ci].civ;
  juntaItem("Estabelecer embaixada", 0, true);
  juntaItem("Roubar uma tecnologia", 1, civ > 0);
  juntaItem("Sabotar a produção", 2, true);
  int custo = 0;
  { RendCidade r = calculaCidade(ci); custo = (CIV[civ].ouro + 1000) * CID[ci].tam / (distanciaCapital(CID[ci]) + 3) + r.escudos * 5; if (temEdif(CID[ci], E_PALACIO)) custo = 999999; }
  char d[16]; sprintf(d, "%d$", custo);
  juntaItem("Incitar uma revolta", 3, custo <= CIV[jogador].ouro, d);
  juntaItem("Cancelar", 4, true);
  listaX0 = 140; listaW = 340; listaLinhas = 6;
  entraModo(M_LISTA);
  desenhaLista();
}

void acaoDiplomata(int op) {
  int i = unidadeAcao, ci = cidadeEm(alvoX, alvoY);
  if (ci < 0 || !U[i].viva) return;
  Cidade & k = CID[ci];
  int civ = k.civ;
  switch (op) {
    case 0:
      if (civ > 0) { CIV[jogador].contacto |= 1 << civ; CIV[civ].contacto |= 1 << jogador; }
      mensagem("Embaixada estabelecida em %s. %s: %s, %d cidades, %ld$ de tesouro.", k.nome, nomePovo(civ),
               GOVERNO[CIV[civ].governo].nome, contaCidades(civ), (long)CIV[civ].ouro);
      apagaUnidade(i);
      break;
    case 1: {
      int cand[N_TECS], n = 0;
      for (int t = 0; t < N_TECS; ++t) if (temTec(civ, t) && !temTec(jogador, t)) cand[n++] = t;
      if (n == 0) mensagem("Os %s não sabem nada que nós não saibamos.", nomePovo(civ));
      else { int t = cand[aleat(n)]; ganhaTec(jogador, t, false); mensagem("O nosso diplomata roubou: %s!", TEC[t].nome); }
      apagaUnidade(i);
      if (civ > 0 && aleat(2)) declaraGuerra(civ, jogador);
      break;
    }
    case 2:
      k.escudos = 0;
      mensagem("Sabotagem em %s: a produção foi destruída.", k.nome);
      apagaUnidade(i);
      if (civ > 0 && aleat(2)) declaraGuerra(civ, jogador);
      break;
    case 3: {
      RendCidade r = calculaCidade(ci);
      int custo = (CIV[civ].ouro + 1000) * k.tam / (distanciaCapital(k) + 3) + r.escudos * 5;
      CIV[jogador].ouro -= custo;
      // as unidades dentro passam para nós
      for (int u = 0; u < MAX_UNIDADES; ++u) if (U[u].viva && U[u].x == k.x && U[u].y == k.y && U[u].civ == civ) { U[u].civ = jogador; U[u].casa = ci; }
      capturaCidade(ci, jogador);
      apagaUnidade(i);
      if (civ > 0) eliminaPovoSeAcabou(civ);
      break;
    }
  }
}

void abreCaravana(int i, int x, int y) {
  unidadeAcao = i; alvoX = x; alvoY = y;
  int ci = cidadeEm(x, y);
  if (ci < 0) return;
  modoAntesLista = M_MAPA;
  char t[60]; snprintf(t, sizeof(t), "Caravana em %s", CID[ci].nome);
  limpaLista(t, L_CARAVANA);
  bool maravilha = CID[ci].civ == jogador && CID[ci].prod >= PROD_MARAV;
  juntaItem("Ajudar a construir a maravilha", 0, maravilha, "+50");
  int d = U[i].casa != SEM && U[i].casa < MAX_CIDADES && CID[U[i].casa].viva ? distancia(CID[U[i].casa].x, CID[U[i].casa].y, x, y) : 0;
  juntaItem("Abrir uma rota de comércio", 1, d >= 9 || CID[ci].civ != jogador);
  juntaItem("Entrar na cidade", 2, CID[ci].civ == jogador);
  juntaItem("Cancelar", 3, true);
  listaX0 = 140; listaW = 340; listaLinhas = 5;
  entraModo(M_LISTA);
  desenhaLista();
}

void acaoCaravana(int op) {
  int i = unidadeAcao, ci = cidadeEm(alvoX, alvoY);
  if (ci < 0 || !U[i].viva) return;
  Cidade & k = CID[ci];
  switch (op) {
    case 0: k.escudos += 50; mensagem("A caravana ajuda %s a construir %s.", k.nome, nomeProd(k.prod)); apagaUnidade(i); break;
    case 1: {
      int casa = U[i].casa;
      int d = casa != SEM && casa < MAX_CIDADES && CID[casa].viva ? distancia(CID[casa].x, CID[casa].y, k.x, k.y) : 10;
      int ganho = (d + 10) * (k.tam + 4) / (k.civ == jogador ? 6 : 3);
      CIV[jogador].ouro += ganho; CIV[jogador].lampadas += ganho;
      if (casa != SEM && casa < MAX_CIDADES && CID[casa].viva && CID[casa].rotaCom < 3) ++CID[casa].rotaCom;
      mensagem("Rota de comércio aberta com %s: %d$ e %d lâmpadas de ciência.", k.nome, ganho, ganho);
      apagaUnidade(i);
      break;
    }
    case 2: U[i].x = k.x; U[i].y = k.y; U[i].mov = 0; break;
  }
}

// ---------------- ações do mapa ----------------
void mudaParaVer(bool liga) {
  modoVer = liga;
  if (liga) {
    if (ativa >= 0 && U[ativa].viva) { cursorX = U[ativa].x; cursorY = U[ativa].y; }
    else { cursorX = embrulhaX(camX + VIEW_W / 2); cursorY = camY + VIEW_H / 2; }
    poeBarra("Modo ver: setas; ESPAÇO acorda as unidades ou abre a cidade.", "ESC ou V: voltar.  F2: unidades a dormir.");
  }
}

void continuaTurno();
bool haChegadaPronta();
void mostraDepoisDeAcao() {
  if (!mostraMensagens()) { entraModo(M_MAPA); desenhaTudo(); if (encontrosPendentes || nTecsNovas || nCenas || haChegadaPronta()) continuaTurno(); }
}

void acabaTurnoJogador();
void continuaTurno();

void ordemColono(int tb) {
  Unidade & u = U[ativa];
  const char * mot = motivoTrabalho(jogador, u.x, u.y, tb);
  if (mot) { poeBarra(mot, ""); somErro(); return; }
  u.estado = EU_TRABALHO; u.trabalho = tb; u.prog = 0; u.mov = 0;
  somOk();
  proximaUnidade();
  desenhaTudo();
}

void executaOrdem(int o) {
  if (o == O_FIM_TURNO) { acabaTurnoJogador(); return; }
  if (o == O_MENU) { abreMenu(); return; }
  if (o == O_VER) { mudaParaVer(true); entraModo(M_MAPA); desenhaTudo(); return; }
  if (ativa < 0 || !U[ativa].viva) { entraModo(M_MAPA); desenhaTudo(); return; }
  Unidade & u = U[ativa];
  entraModo(M_MAPA);
  switch (o) {
    case O_FUNDAR: {
      const char * mot = motivoFundar(jogador, u.x, u.y);
      if (mot) { poeBarra(mot, ""); somErro(); desenhaTudo(); return; }
      int ci = fundaCidade(jogador, u.x, u.y);
      apagaUnidade(ativa);
      ativa = -1;
      if (ci >= 0) { somTri(784, 200, 50); abreNomeCidade(ci); return; }
      break;
    }
    case O_JUNTAR: {
      int ci = cidadeEm(u.x, u.y);
      if (ci >= 0 && CID[ci].tam < 8) { ++CID[ci].tam; arrumaCidade(ci); apagaUnidade(ativa); ativa = -1; }
      break;
    }
    case O_ESTRADA:   ordemColono(TB_ESTRADA); return;
    case O_FERROVIA:  ordemColono(TB_FERROVIA); return;
    case O_REGA:      ordemColono(TB_REGA); return;
    case O_MINA:      ordemColono(TB_MINA); return;
    case O_FORTALEZA: ordemColono(TB_FORTALEZA); return;
    case O_LIMPAR:    ordemColono(TB_LIMPAR); return;
    case O_FORTIFICAR: u.estado = EU_FORTIFICANDO; u.mov = 0; break;
    case O_SENTINELA:  u.estado = EU_SENTINELA; break;
    case O_IR:
      modoVer = true; cursorX = u.x; cursorY = u.y;
      entraModo(M_DESTINO);
      desenhaTudo();
      poeBarra("Ir para: escolhe o destino com as setas e ESPAÇO.", "C: lista das nossas cidades.  ESC: cancelar.");
      return;
    case O_ATACAR: abreAlvos(); return;
    case O_ESPERAR: { int a = ativa; proximaUnidade(); if (ativa == a) ativa = a; break; }
    case O_NADA: u.mov = 0; break;
    case O_CASA: { int ci = cidadeEm(u.x, u.y); if (ci >= 0) u.casa = ci; break; }
    case O_DISPENSAR: apagaUnidade(ativa); ativa = -1; break;
    case O_VER_CIDADE: { int ci = cidadeEm(u.x, u.y); if (ci >= 0) { abreCidade(ci); return; } break; }
    case O_ACORDAR: {
      int lista[16], n = unidadesEm(u.x, u.y, lista, 16);
      for (int k = 0; k < n; ++k) if (U[lista[k]].civ == jogador && U[lista[k]].estado != EU_TRABALHO) U[lista[k]].estado = EU_NADA;
      // num navio no mar: passa logo para o primeiro passageiro que ainda se pode mexer, para desembarcar
      if (eOceano(u.x, u.y) && UNIDADE[u.tipo].dominio == D_MAR)
        for (int k = 0; k < n; ++k) {
          const Unidade & p = U[lista[k]];
          if (p.civ == jogador && UNIDADE[p.tipo].dominio == D_TERRA && p.mov > 0) { ativa = lista[k]; break; }
        }
      break;
    }
  }
  if (ativa < 0 || !U[ativa].viva || !unidadeEsperaOrdens(ativa)) proximaUnidade();
  desenhaTudo();
}

// pequena explosão na casa (x, y) quando há combate
void explosao(int x, int y) {
  int vx = difX(camX, x), vy = y - camY;
  if (vx < 0 || vx >= VIEW_W || vy < 0 || vy >= VIEW_H) return;
  int cx = vx * TILE_W + TILE_W / 2, cy = vy * TILE_H + TILE_H / 2;
  const RGB888 cores[3] = { C_AMARELO, C_OURO, C_VERMELHO };
  for (int k = 0; k < 6; ++k) {
    int r = 3 + k * 2;
    canvas.setBrushColor(cores[k % 3]);
    canvas.fillEllipse(cx, cy, r * 2, r);
    canvas.waitCompletion(false);
    delay(35);
  }
  redesenhaCasa(x, y);
}

void tentaMover(int d) {
  if (ativa < 0 || !U[ativa].viva) return;
  int i = ativa;
  int ox = U[i].x, oy = U[i].y;
  int alvoXm = U[i].x + DIR_DX[d], alvoYm = U[i].y + DIR_DY[d];
  ResMov r = moveUnidade(i, d);
  if (r == MV_GANHOU || r == MV_PERDEU || r == MV_DESTRUIU) explosao(alvoXm, alvoYm);
  switch (r) {
    case MV_OK: case MV_EMBARCOU: somPasso(); break;
    case MV_GANHOU: somRuido(150, 60); poeBarra("Vitória!", ""); break;
    case MV_PERDEU: som(120, 300, 40); poeBarra("A nossa unidade foi derrotada.", ""); break;
    case MV_CONQUISTOU: somTri(660, 300, 60); break;
    case MV_DESTRUIU: somRuido(600, 80); break;
    case MV_ZOC: somErro(); poeBarra("Zona de controlo inimiga: não se pode passar entre unidades estrangeiras.", ""); return;
    case MV_SEM_LUGAR: somErro(); poeBarra("Não há lugar nos navios.", ""); return;
    case MV_BLOQUEADO: somErro(); return;
    case MV_SEM_MOV: break;
    case MV_PAZ: {
      int nx = U[i].x + DIR_DX[d], ny = U[i].y + DIR_DY[d];
      int c = donoDaCasa(nx, ny);
      perguntaDir = d;
      pergunta(Q_GUERRA, "Estamos em paz com os %s. Atacar quebra o tratado: declarar guerra?", nomePovo(c));
      return;
    }
    case MV_DIPLOMATA: abreDiplomata(i, U[i].x + DIR_DX[d], U[i].y + DIR_DY[d]); return;
    case MV_CARAVANA:  abreCaravana(i, U[i].x + DIR_DX[d], U[i].y + DIR_DY[d]); return;
  }
  (void)ox; (void)oy;
  if (!U[i].viva || U[i].mov == 0 || U[i].estado != EU_NADA) proximaUnidade();
  if (ativa >= 0 && !naVista(U[ativa].x, U[ativa].y, 2)) centraEm(U[ativa].x, U[ativa].y);
  if (nMsg) { desenhaTudo(); mostraMensagens(); return; }
  desenhaTudo();
  if (encontrosPendentes || nTecsNovas || nCenas || haChegadaPronta()) continuaTurno();   // encontro, descoberta, cena ou ataque a confirmar
}

// direção pedida pelo jogador neste fotograma (-1 = nenhuma)
int direcaoPedida() {
  for (int k = 0; k < nTeclas; ++k) {
    switch (filaTeclas[k]) {
      case VK_UP: case VK_KP_UP: case VK_KP_8: return 0;
      case VK_PAGEUP: case VK_KP_PAGEUP: case VK_KP_9: return 1;
      case VK_RIGHT: case VK_KP_RIGHT: case VK_KP_6: return 2;
      case VK_PAGEDOWN: case VK_KP_PAGEDOWN: case VK_KP_3: return 3;
      case VK_DOWN: case VK_KP_DOWN: case VK_KP_2: return 4;
      case VK_END: case VK_KP_END: case VK_KP_1: return 5;
      case VK_LEFT: case VK_KP_LEFT: case VK_KP_4: return 6;
      case VK_HOME: case VK_KP_HOME: case VK_KP_7: return 7;
      default: break;
    }
  }
  // joystick (com diagonais), com repetição
  static int rep = 0, antes = -1;
  int jd = -1;
  bool c = joy & 1, b = joy & 2, e = joy & 4, d = joy & 8;
  if (c && d) jd = 1; else if (b && d) jd = 3; else if (b && e) jd = 5; else if (c && e) jd = 7;
  else if (c) jd = 0; else if (d) jd = 2; else if (b) jd = 4; else if (e) jd = 6;
  if (jd < 0) { rep = 0; antes = -1; return -1; }
  if (jd != antes) { antes = jd; rep = 0; }
  ++rep;
  if (rep == 3 || (rep > 18 && rep % 8 == 0)) return jd;   // espera 3 fotogramas para apanhar as diagonais
  return -1;
}

bool joyFogo() { static bool antes = false; bool f = joy & 0x10; bool r = f && !antes; antes = f; return r; }
bool joyVolta() { static bool antes = false; bool f = joy & 0x20; bool r = f && !antes; antes = f; return r; }

void atualizaMapa() {
  // piscar a unidade ativa
  if (!modoVer && ativa >= 0 && U[ativa].viva && frame % 15 == 0) {
    piscaApagado = !piscaApagado;
    int vx = difX(camX, U[ativa].x), vy = U[ativa].y - camY;
    if (vx >= 0 && vx < VIEW_W && vy >= 0 && vy < VIEW_H) {
      if (piscaApagado) { desenhaCasa(vx * TILE_W, vy * TILE_H, U[ativa].x, U[ativa].y, false); moldura(); }
      else redesenhaCasa(U[ativa].x, U[ativa].y);
    }
  }
  int d = direcaoPedida();
  bool enter = premiu(VK_RETURN) || premiu(VK_KP_ENTER) || joyFogo();
  bool esc = premiu(VK_ESCAPE) || joyVolta();
  if (modoVer) {
    if (d >= 0) {
      cursorX = embrulhaX(cursorX + DIR_DX[d]); cursorY = constrain(cursorY + DIR_DY[d], 0, MAPA_H - 1);
      if (!naVista(cursorX, cursorY, 1)) centraEm(cursorX, cursorY);
      desenhaMapa(); desenhaPainel();
    }
    if (esc || premiuLetra('V')) { modoVer = false; proximaUnidade(); desenhaTudo(); poeBarra("", ""); return; }
    if (enter || premiu(VK_SPACE)) {
      int ci = cidadeEm(cursorX, cursorY);
      if (ci >= 0 && CID[ci].civ == jogador) { abreCidade(ci); return; }
      // acorda uma unidade nossa nesta casa
      int lista[16], n = unidadesEm(cursorX, cursorY, lista, 16);
      for (int k = 0; k < n; ++k) if (U[lista[k]].civ == jogador) {
        U[lista[k]].estado = EU_NADA;
        if (U[lista[k]].mov > 0) { ativa = lista[k]; modoVer = false; desenhaTudo(); return; }
      }
      somErro();
    }
    return;
  }
  if (d >= 0) {
    if (ativa >= 0) tentaMover(d);
    else { mudaParaVer(true); }
    return;
  }
  if (esc) { abreMenu(); return; }
  if (enter) {
    if (ativa < 0) acabaTurnoJogador();
    else abreOrdens();
    return;
  }
  if (premiu(VK_SPACE) && ativa >= 0) { executaOrdem(O_NADA); return; }
  if (premiu(VK_F1)) { abreListaCidades(); return; }
  if (premiu(VK_F2)) { abreDormir(); return; }
  if (premiu(VK_F3)) { abreRelatorio(MN_POVOS); return; }
  if (premiu(VK_F6)) { abreRelatorio(MN_CIENCIA); return; }
  if (premiu(VK_F7)) { abreRelatorio(MN_MARAVILHAS); return; }
  if (premiu(VK_F9)) { abreRelatorio(MN_PONTOS); return; }
  if (premiu(VK_F10)) {                     // grava na mesma posição da última vez; se não houver, pergunta
    if (ultimaGravacao < 0) { abreGravar(); return; }
    poeBarra(gravaJogo(ultimaGravacao) ? "Jogo gravado no cartão SD." : "Não foi possível gravar (cartão SD?).", ""); return;
  }
  if (premiuLetra('T')) { taxaSel = 0; entraModo(M_TAXAS); desenhaTaxas(); return; }
  if (premiuLetra('X')) { if (CIV[jogador].anarquia == 0) abreGoverno(); return; }
  if (premiuLetra('Y')) { abreDiplomacia(); return; }
  if (premiuLetra('V')) { mudaParaVer(true); desenhaTudo(); poeBarra("Modo ver: setas; ESPAÇO acorda as unidades ou abre a cidade.", "ESC ou V: voltar.  F2: unidades a dormir."); return; }
  if (ativa < 0) return;
  Unidade & u = U[ativa];
  bool colono = UNIDADE[u.tipo].flags & U_COLONO;
  if (premiuLetra('B') && colono) { executaOrdem(cidadeEm(u.x, u.y) >= 0 ? O_JUNTAR : O_FUNDAR); return; }
  if (premiuLetra('R') && colono) { executaOrdem((melh(u.x, u.y) & M_ESTRADA) ? O_FERROVIA : O_ESTRADA); return; }
  if (premiuLetra('I') && colono) { executaOrdem(O_REGA); return; }
  if (premiuLetra('M') && colono) { executaOrdem(O_MINA); return; }
  if (premiuLetra('P') && colono) { executaOrdem(O_LIMPAR); return; }
  if (premiuLetra('F')) { executaOrdem(colono ? O_FORTALEZA : O_FORTIFICAR); return; }
  if (premiuLetra('S')) { executaOrdem(O_SENTINELA); return; }
  if (premiuLetra('G') || (premiuLetra('M') && !colono)) { executaOrdem(O_IR); return; }
  if (premiuLetra('A')) { abreAlvos(); return; }
  if (premiuLetra('W')) { executaOrdem(O_ESPERAR); return; }
  if (premiuLetra('H')) { executaOrdem(O_CASA); return; }
  if (premiuLetra('D')) { executaOrdem(O_DISPENSAR); return; }
  if (premiuLetra('U')) { executaOrdem(O_ACORDAR); return; }
  if (premiuLetra('C')) {
    int ci = cidadeEm(u.x, u.y);
    if (ci >= 0 && CID[ci].civ == jogador) abreCidade(ci);
    else { centraEm(u.x, u.y); desenhaTudo(); }
    return;
  }
}

// "ir para" uma das nossas cidades: lista com as distâncias
void abreIrCidade() {
  if (ativa < 0 || !U[ativa].viva) return;
  const Unidade & u = U[ativa];
  const InfoUnidade & I = UNIDADE[u.tipo];
  modoAntesLista = M_DESTINO;
  char t[64]; snprintf(t, sizeof(t), "%s: ir para que cidade?", I.nome);
  limpaLista(t, L_IR_CIDADE);
  int cand[MAX_CIDADES], dist[MAX_CIDADES], n = 0;
  for (int ci = 0; ci < MAX_CIDADES; ++ci) {
    const Cidade & k = CID[ci];
    if (!k.viva || k.civ != jogador) continue;
    cand[n] = ci; dist[n] = distancia(u.x, u.y, k.x, k.y); ++n;
  }
  for (int a = 0; a < n; ++a) for (int b = a + 1; b < n; ++b) if (dist[b] < dist[a]) { int x = dist[a]; dist[a] = dist[b]; dist[b] = x; x = cand[a]; cand[a] = cand[b]; cand[b] = x; }
  for (int j = 0; j < n; ++j) {
    const Cidade & k = CID[cand[j]];
    // por terra só se chega às cidades do mesmo continente; os navios só às costeiras
    bool chega = true;
    const char * porque = "";
    if (I.dominio == D_TERRA && continente(k.x, k.y) != continente(u.x, u.y)) { chega = false; porque = " (outro continente)"; }
    if (I.dominio == D_MAR && !juntoAoMar(k.x, k.y)) { chega = false; porque = " (não tem porto)"; }
    if (k.x == u.x && k.y == u.y) { chega = false; porque = " (já está aqui)"; }
    char nome[44], d[16];
    snprintf(nome, sizeof(nome), "%-13s %2d%s", k.nome, k.tam, porque);
    snprintf(d, sizeof(d), "%d casa%s", dist[j], dist[j] == 1 ? "" : "s");
    juntaItem(nome, cand[j], chega, d);
  }
  if (n == 0) { somErro(); poeBarra("Ainda não temos cidades.", "ESC: cancelar."); return; }
  for (int j = 0; j < nItens; ++j) if (itens[j].ativo) { itemSel = j; break; }
  strcpy(ajudaLista, "ESPAÇO: ir para lá   ESC: escolher no mapa");
  listaX0 = 80; listaW = 380; listaLinhas = 14;
  entraModo(M_LISTA);
  desenhaLista();
}

// ---- unidades a dormir: lista das que estão de sentinela ou fortificadas, para as acordar sem rato ----
const int DORMIR_TODAS = 30000;
void abreDormir() {
  modoAntesLista = M_MAPA;
  limpaLista("Unidades a dormir: qual acordamos?", L_DORMIR);
  int cand[MAX_UNIDADES], ord[MAX_UNIDADES], n = 0, fora = 0;
  for (int i = 0; i < MAX_UNIDADES; ++i) {
    const Unidade & u = U[i];
    if (!u.viva || u.civ != jogador) continue;
    if (u.estado != EU_SENTINELA && u.estado != EU_FORTIFICANDO && u.estado != EU_FORTIFICADA && u.estado != EU_A_BORDO) continue;
    int ci = cidadeEm(u.x, u.y);
    if (ci < 0) ++fora;
    // primeiro as que estão fora das cidades, depois por cidade mais próxima
    int perto = cidadeMaisPerto(jogador, u.x, u.y);
    cand[n] = i; ord[n] = (ci >= 0 ? 100000 : 0) + (perto >= 0 ? perto * 100 + distancia(u.x, u.y, CID[perto].x, CID[perto].y) : 99); ++n;
  }
  for (int a = 0; a < n; ++a) for (int b = a + 1; b < n; ++b) if (ord[b] < ord[a]) { int x = ord[a]; ord[a] = ord[b]; ord[b] = x; x = cand[a]; cand[a] = cand[b]; cand[b] = x; }
  if (n == 0) { somErro(); entraModo(M_MAPA); desenhaTudo(); poeBarra("Não há unidades a dormir.", ""); return; }
  if (fora > 1) { char t[48]; snprintf(t, sizeof(t), "Acordar todas fora das cidades (%d)", fora); juntaItem(t, DORMIR_TODAS, true); }
  for (int j = 0; j < n && nItens < MAX_ITENS; ++j) {
    const Unidade & u = U[cand[j]];
    const char * est = u.estado == EU_SENTINELA ? "sentinela" : u.estado == EU_A_BORDO ? "a bordo" : "fortif.";
    if (u.estado == EU_SENTINELA && eOceano(u.x, u.y) && UNIDADE[u.tipo].dominio == D_TERRA) est = "a bordo";
    int ci = cidadeEm(u.x, u.y), perto = cidadeMaisPerto(jogador, u.x, u.y);
    char t[60], d[16];
    snprintf(t, sizeof(t), "%-12s %-9s %s", UNIDADE[u.tipo].nome, est, ci >= 0 ? CID[ci].nome : perto >= 0 ? CID[perto].nome : "");
    if (ci >= 0) snprintf(d, sizeof(d), "na cidade");
    else if (perto >= 0) { int dd = distancia(u.x, u.y, CID[perto].x, CID[perto].y); snprintf(d, sizeof(d), "a %d casa%s", dd, dd == 1 ? "" : "s"); }
    else d[0] = 0;
    juntaItem(t, cand[j], true, d[0] ? d : nullptr);
  }
  strcpy(ajudaLista, "ESPAÇO: acordar e ir para ela   ESC: voltar");
  listaX0 = 60; listaW = 420; listaLinhas = 15;
  entraModo(M_LISTA);
  desenhaLista();
}
// acorda a unidade i (ou todas as de fora das cidades) e passa para ela
void acordaDormir(int v) {
  modoVer = false;
  entraModo(M_MAPA);
  if (v == DORMIR_TODAS) {
    int primeira = -1;
    for (int i = 0; i < MAX_UNIDADES; ++i) {
      Unidade & u = U[i];
      if (!u.viva || u.civ != jogador || cidadeEm(u.x, u.y) >= 0) continue;
      if (u.estado != EU_SENTINELA && u.estado != EU_FORTIFICANDO && u.estado != EU_FORTIFICADA && u.estado != EU_A_BORDO) continue;
      u.estado = EU_NADA;
      if (primeira < 0 && u.mov > 0) primeira = i;
    }
    if (primeira >= 0) { ativa = primeira; centraEm(U[ativa].x, U[ativa].y); }
    desenhaTudo();
    return;
  }
  if (v < 0 || v >= MAX_UNIDADES || !U[v].viva) { desenhaTudo(); return; }
  Unidade & u = U[v];
  u.estado = EU_NADA;
  centraEm(u.x, u.y);
  if (u.mov > 0) { ativa = v; desenhaTudo(); }
  else { desenhaTudo(); poeBarra("Acordada. Já não tem movimento: mexe-se no próximo turno.", ""); }
}

void atualizaDestino() {
  int d = direcaoPedida();
  if (d >= 0) {
    cursorX = embrulhaX(cursorX + DIR_DX[d]); cursorY = constrain(cursorY + DIR_DY[d], 0, MAPA_H - 1);
    if (!naVista(cursorX, cursorY, 1)) centraEm(cursorX, cursorY);
    desenhaMapa();
  }
  if (premiu(VK_ESCAPE) || joyVolta()) { modoVer = false; entraModo(M_MAPA); desenhaTudo(); poeBarra("", ""); return; }
  if (premiuLetra('C')) { abreIrCidade(); return; }
  if (premiu(VK_RETURN) || premiu(VK_SPACE) || premiu(VK_KP_ENTER) || joyFogo()) {
    modoVer = false;
    entraModo(M_MAPA);
    if (ativa >= 0 && U[ativa].viva) {
      Unidade & u = U[ativa];
      u.gx = cursorX; u.gy = cursorY; u.estado = EU_DESTINO;
      segueDestino(ativa);
      proximaUnidade();
    }
    poeBarra("", "");
    if (nMsg) { desenhaTudo(); mostraMensagens(); return; }
    desenhaTudo();
  }
}

// ---------------- o turno ----------------
void comecaTurnoJogador();

bool verificaFim() {
  if (fimDeJogo) return true;
  if (!CIV[jogador].viva) { fimDeJogo = 2; return true; }
  bool outros = false;
  for (int c = 1; c < N_CIVS; ++c) if (c != jogador && CIV[c].viva) outros = true;
  if (!outros) { fimDeJogo = 1; return true; }
  for (int c = 1; c < N_CIVS; ++c) {
    Civ & C = CIV[c];
    if (!C.viva || C.naveChega == 0) continue;
    if (C.capital == SEM) { C.naveChega = 0; C.nave[0] = C.nave[1] = C.nave[2] = 0; mensagem("A nave dos %s perdeu-se: a capital caiu.", nomePovo(c)); continue; }
    if (turno >= C.naveChega) { fimDeJogo = c == jogador ? 1 : 2; return true; }
  }
  if (turno >= TURNO_FIM) {
    int melhor = 0, mp = -1;
    for (int c = 1; c < N_CIVS; ++c) if (CIV[c].viva) { int p = calculaPontos(c); if (p > mp) { mp = p; melhor = c; } }
    fimDeJogo = melhor == jogador ? 1 : 2;
    return true;
  }
  return false;
}

void desenhaFim();
// ---------------- movimentos dos outros povos (opção do menu, como "Enemy Moves" no original) ----------------
bool turnoDosOutros = false;        // a IA está a jogar
bool saltaMovIA = false;            // o jogador carregou numa tecla: não mostrar mais neste turno

// o jogador vê agora esta casa? (ao lado das suas unidades ou a 2 casas das suas cidades)
bool jogadorVe(int x, int y) {
  for (int k = 0; k < MAX_UNIDADES; ++k) if (U[k].viva && U[k].civ == jogador && distancia(U[k].x, U[k].y, x, y) <= 1) return true;
  for (int k = 0; k < MAX_CIDADES; ++k) if (CID[k].viva && CID[k].civ == jogador && distancia(CID[k].x, CID[k].y, x, y) <= 2) return true;
  return false;
}
// espera ms milissegundos com a música a tocar; ESPAÇO, ESC ou ENTER saltam o resto dos movimentos
void esperaMovIA(int ms) {
  canvas.waitCompletion(false);
  uint32_t fim = millis() + ms;
  while ((int32_t)(millis() - fim) < 0) {
    leEntrada();
    atualizaMusica(millis());
    if (premiu(VK_SPACE) || premiu(VK_ESCAPE) || premiu(VK_RETURN) || joyFogo()) saltaMovIA = true;
    delay(4);
  }
}
// redesenha as casas entre (ox, oy) e (nx, ny); 'semUnidade' = casa desenhada só com o terreno
void redesenhaPar(int ox, int oy, int nx, int ny, int semX, int semY) {
  const int xs[4] = { ox, nx, ox, nx }, ys[4] = { oy, ny, ny, oy };
  for (int k = 0; k < 4; ++k) {
    bool repetida = false;
    for (int j = 0; j < k; ++j) if (xs[j] == xs[k] && ys[j] == ys[k]) repetida = true;
    int vx = difX(camX, xs[k]), vy = ys[k] - camY;
    if (repetida || vx < 0 || vx >= VIEW_W || vy < 0 || vy >= VIEW_H) continue;
    bool sem = embrulhaX(xs[k]) == embrulhaX(semX) && ys[k] == semY;
    desenhaCasa(vx * TILE_W, vy * TILE_H, xs[k], ys[k], !sem);
  }
}
void animaIA(int i, int civ, int ox, int oy, int nx, int ny, int fase) {
  static bool mostrouAtaque = false;            // o fim do combate mostra-se sempre que se mostrou o início
  if (fase == 2) {
    if (!mostrouAtaque) return;
    mostrouAtaque = false;
    if (!naVista(ox, oy, 0) || !naVista(nx, ny, 0)) return;
    calculaVistoAgora();
  } else {
    if (!verInimigos || !turnoDosOutros || saltaMovIA || civ == jogador) return;
    if (!jogadorVe(ox, oy) && !jogadorVe(nx, ny)) return;
  }
  if (fase == 2) {
    canvas.setClippingRect(Rect(0, 0, MAPA_PX_W - 1, MAPA_PX_H - 1));
    redesenhaPar(ox, oy, nx, ny, -1000, -1000);
    canvas.setClippingRect(Rect(0, 0, ECRA_W - 1, ECRA_H - 1));
    desenhaNomesCidades(); moldura();
    esperaMovIA(350);
    return;
  }
  // a vista acompanha a unidade
  if (!naVista(ox, oy, 0) || !naVista(nx, ny, 0)) {
    centraEm(nx, ny);
    desenhaMapa(); desenhaPainel();
    poeBarra("Os outros povos estão a jogar...", "ESPAÇO: não mostrar mais");
  } else calculaVistoAgora();
  if (!naVista(ox, oy, 0) || !naVista(nx, ny, 0)) return;
  const int ax = difX(camX, ox) * TILE_W, ay = (oy - camY) * TILE_H;
  const int bx = difX(camX, nx) * TILE_W, by = (ny - camY) * TILE_H;
  canvas.setClippingRect(Rect(0, 0, MAPA_PX_W - 1, MAPA_PX_H - 1));
  switch (fase) {
    case 0: {                                    // desliza da casa de partida para a de chegada
      const int N = 5;
      for (int f = 1; f <= N && !saltaMovIA; ++f) {
        redesenhaPar(ox, oy, nx, ny, nx, ny);
        if (f < N) desenhaUnidade(ax + (bx - ax) * f / N, ay + (by - ay) * f / N, i, 1);
        else redesenhaPar(ox, oy, nx, ny, -1000, -1000);
        esperaMovIA(28);
      }
      redesenhaPar(ox, oy, nx, ny, -1000, -1000);
      break;
    }
    case 1: {                                    // investe duas vezes contra o alvo
      mostrouAtaque = true;
      somTri(140, 90, 70);
      for (int r = 0; r < 2 && !saltaMovIA; ++r)
        for (int f = 0; f <= 4; ++f) {
          int q = f <= 2 ? f : 4 - f;            // 0, 1, 2, 1, 0
          redesenhaPar(ox, oy, nx, ny, ox, oy);
          desenhaUnidade(ax + (bx - ax) * q / 5, ay + (by - ay) * q / 5, i, 1);
          esperaMovIA(30);
        }
      break;
    }
  }
  canvas.setClippingRect(Rect(0, 0, ECRA_W - 1, ECRA_H - 1));
  desenhaNomesCidades();
  moldura();
}

void acabaTurnoJogador() {
  ativa = -1;
  poeBarra("Os outros povos estão a jogar...", verInimigos ? "ESPAÇO: não mostrar mais" : "");
  canvas.waitCompletion(false);
  turnoDosOutros = true; saltaMovIA = false;
  for (int c = 1; c <= N_CIVS; ++c) {
    int civ = c % N_CIVS;                 // 1..7 e depois os bárbaros (0)
    if (civ == jogador || !CIV[civ].viva) continue;
    inicioTurnoCiv(civ);
    turnoIA(civ);
    // a IA lança a nave quando a tem completa
    if (civ > 0 && CIV[civ].naveChega == 0 && CIV[civ].nave[0] >= 8 && CIV[civ].nave[1] >= 4 && CIV[civ].nave[2] >= 2) {
      CIV[civ].naveChega = turno + 12;
      mensagem("Os %s lançaram uma nave espacial para Alfa do Centauro! Chega dentro de 12 turnos.", nomePovo(civ));
    }
  }
  turnoDosOutros = false;
  ++turno;
  if (turno % 10 == 0) { Serial.printf("[jogo] turno %d:", turno); for (int c = 1; c < N_CIVS; ++c) if (CIV[c].viva) Serial.printf(" %d:%dc/%du", c, contaCidades(c), contaUnidades(c)); Serial.printf(" bárb %du\n", contaUnidades(0)); }
  if (verificaFim()) { desenhaFim(); return; }
  if (turno % 10 == 0 && temSD) gravaJogo(GRAV_AUTO);     // gravação automática, à parte das 10 posições
  comecaTurnoJogador();
}

void comecaTurnoJogador() {
  inicioTurnoCiv(jogador);
  noTitulo = false;
  { int n = CIV[jogador].nTecs; epocaMusical = n < 15 ? 0 : n < 35 ? 1 : n < 55 ? 2 : 3; }
  // ir para: as unidades seguem o caminho
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].civ == jogador && U[i].estado == EU_DESTINO) segueDestino(i);
  // as sentinelas acordam se houver estrangeiros ao lado
  for (int i = 0; i < MAX_UNIDADES; ++i) {
    Unidade & u = U[i];
    if (!u.viva || u.civ != jogador || u.estado != EU_SENTINELA) continue;
    for (int d = 0; d < 8; ++d) { int c = civNaCasa(u.x + DIR_DX[d], u.y + DIR_DY[d]); if (c >= 0 && c != jogador) { u.estado = EU_NADA; break; } }
  }
  if (verificaFim()) { desenhaFim(); return; }
  ativa = -1; ultimaAtiva = -1;
  proximaUnidade();
  entraModo(M_MAPA);
  desenhaTudo();
  poeBarra("", "");
  continuaTurno();
}

// ---------------- primeiro contacto: o líder aparece e oferece a paz (como no original) ----------------
int civEncontro = 0, encontroSel = 0;
void desenhaOpcoesEncontro() {
  const char * op[2] = { "Aceitar a paz", "Recusar (guerra)" };
  for (int i = 0; i < 2; ++i) {
    int bx = 180 + i * 160;
    retangulo(bx - 6, 182, bx + 134, 193, i == encontroSel ? C_MOLDURA : C_PAINEL);
    textoCentro(bx - 6, bx + 134, 183, op[i], i == encontroSel ? C_AMARELO : C_BRANCO);
  }
  moldura();
}
void desenhaEncontro() {
  const int c = civEncontro, cor = corDe(c);
  const RGB888 cc = corCiv(cor), ce = corCivE(cor);
  const RGB888 vinho(85, 0, 0), pedra(170, 170, 170), pedraE(85, 85, 85), ouroE(170, 85, 0);
  limpaEcra(C_PRETO);
  char b[200];
  snprintf(b, sizeof(b), "Primeiro contacto com os %s", nomePovo(c));
  textoCentro(0, ECRA_W - 1, 2, b, C_AMARELO);
  // sala do trono: parede, pilastras, cortinas na cor do povo, sanefa dourada e chão
  retangulo(1, 12, ECRA_W - 2, 105, vinho);
  for (int y = 16; y < 104; y += 8)
    for (int x = 1 + ((y / 8) & 1) * 12; x < ECRA_W - 2; x += 24) retangulo(x, y, x + 1, y + 1, RGB888(170, 0, 0));
  const int pil[2] = { 168, 452 };
  for (int k = 0; k < 2; ++k) {
    int x = pil[k];
    retangulo(x, 18, x + 19, 103, pedra);
    retangulo(x + 14, 18, x + 19, 103, pedraE);
    retangulo(x + 4, 18, x + 5, 103, C_BRANCO);
    retangulo(x - 3, 18, x + 22, 22, C_BRANCO); retangulo(x - 3, 98, x + 22, 103, C_BRANCO);
  }
  for (int x = 1; x < 150; x += 9) {                       // cortinas com pregas
    int r = ECRA_W - 2 - (x - 1) - 8;
    retangulo(x, 12, x + 5, 105, cc); retangulo(x + 6, 12, x + 8, 105, ce); retangulo(x + 1, 12, x + 1, 105, C_BRANCO);
    retangulo(r, 12, r + 5, 105, cc); retangulo(r + 6, 12, r + 8, 105, ce); retangulo(r + 1, 12, r + 1, 105, C_BRANCO);
  }
  retangulo(1, 12, ECRA_W - 2, 17, C_OURO);
  for (int x = 1; x < ECRA_W - 2; x += 12) retangulo(x + 2, 18, x + 8, 20, C_OURO), retangulo(x + 4, 21, x + 6, 22, ouroE);
  // encosto do trono por trás do retrato
  canvas.setBrushColor(C_OURO);  canvas.fillEllipse(320, 30, 200, 26);
  retangulo(220, 30, 419, 105, C_OURO);
  canvas.setBrushColor(RGB888(170, 0, 0)); canvas.fillEllipse(320, 32, 184, 20);
  retangulo(228, 32, 411, 105, RGB888(170, 0, 0));
  retangulo(415, 30, 419, 105, ouroE);
  for (int y = 106; y < 112; y += 2)                        // chão em xadrez
    for (int x = 1; x < ECRA_W - 2; x += 16) retangulo(x + ((y / 2) & 1) * 8, y, x + ((y / 2) & 1) * 8 + 7, y + 1, pedraE);
  desenhaArte(240, 22, dadosRetrato(CIV[c].povo % N_RETRATOS), RETRATO_W, RETRATO_H);
  // o que o líder diz
  caixa(4, 113, ECRA_W - 5, 197, C_PAINEL, C_MOLDURA);
  snprintf(b, sizeof(b), "%s, líder dos %s", POVO[CIV[c].povo].lider, nomePovo(c));
  textoCentro(4, ECRA_W - 5, 118, b, cc);
  const char * tom[3] = {
    "O nosso povo ama a paz e o comércio.",
    "Respeitamos quem nos respeita.",
    "Os nossos exércitos são temidos em toda a parte, mas hoje estendemos-vos a mão." };
  snprintf(b, sizeof(b), "\"Saudações, %s, chefe dos %s! %s Proponho que os nossos povos vivam em paz. Aceitas?\"",
           nomeLider(jogador), nomePovo(jogador), tom[POVO[CIV[c].povo].agressivo % 3]);
  textoQuebrado(16, 132, 74, b, C_BRANCO);
  desenhaOpcoesEncontro();
}
void abreEncontro(int c) {
  civEncontro = c; encontroSel = 0;
  entraModo(M_ENCONTRO);
  TOCA(MUS_TITULO, BAI_TITULO, 120, false);
  desenhaEncontro();
}
void respondeEncontro(bool paz) {
  int c = civEncontro;
  entraModo(M_MAPA);
  if (paz) {
    fazPaz(jogador, c);
    mensagem("Paz com os %s! %s promete respeitar as nossas fronteiras.", nomePovo(c), POVO[CIV[c].povo].lider);
  } else declaraGuerra(jogador, c);
  desenhaTudo();
  continuaTurno();
}
void atualizaEncontro() {
  if (menuEsq() || menuDir() || menuCima() || menuBaixo()) { encontroSel ^= 1; somMenu(); desenhaOpcoesEncontro(); }
  if (premiuLetra('S') || premiuLetra('A')) { somOk(); respondeEncontro(true); return; }
  if (premiuLetra('N') || premiuLetra('R')) { somOk(); respondeEncontro(false); return; }
  if (tempo > 10 && carregouFogo()) { somOk(); respondeEncontro(encontroSel == 0); return; }
}

// ---------------- marcha contra uma cidade inimiga (tecla A) ----------------
bool haChegadaPronta() {
  for (int k = 0; k < nChegadas; ++k) { int i = chegouUnid[k]; if (U[i].viva && U[i].mov > 0) return true; }
  return false;
}
bool podeMarchar(int i) {
  const Unidade & u = U[i];
  return u.viva && eMilitar(u.tipo) && !(UNIDADE[u.tipo].flags & U_NUCLEAR);
}
// cidades estrangeiras conhecidas que a unidade pode alcançar, da mais perto para a mais longe
void abreAlvos() {
  if (ativa < 0 || !podeMarchar(ativa)) { somErro(); poeBarra("Só as unidades militares podem marchar contra uma cidade.", ""); return; }
  const Unidade & u = U[ativa];
  const InfoUnidade & I = UNIDADE[u.tipo];
  modoAntesLista = M_MAPA;
  char t[64]; snprintf(t, sizeof(t), "%s: atacar que cidade?", I.nome);
  limpaLista(t, L_ALVOS);
  int cand[MAX_CIDADES], dist[MAX_CIDADES], n = 0;
  for (int ci = 0; ci < MAX_CIDADES; ++ci) {
    const Cidade & k = CID[ci];
    if (!k.viva || k.civ == jogador || !viu(jogador, k.x, k.y)) continue;
    if (I.dominio == D_TERRA && continente(k.x, k.y) != continente(u.x, u.y)) continue;   // por terra, só no mesmo continente
    if (I.dominio == D_MAR && !juntoAoMar(k.x, k.y)) continue;
    cand[n] = ci; dist[n] = distancia(u.x, u.y, k.x, k.y); ++n;
  }
  for (int a = 0; a < n; ++a) for (int b = a + 1; b < n; ++b) if (dist[b] < dist[a]) { int x = dist[a]; dist[a] = dist[b]; dist[b] = x; x = cand[a]; cand[a] = cand[b]; cand[b] = x; }
  for (int j = 0; j < n; ++j) {
    const Cidade & k = CID[cand[j]];
    char nome[44], d[16];
    snprintf(nome, sizeof(nome), "%-13s %s%s", k.nome, nomePovo(k.civ), k.civ == 0 ? "" : (emGuerra(jogador, k.civ) ? " (guerra)" : " (paz)"));
    snprintf(d, sizeof(d), "%d casa%s", dist[j], dist[j] == 1 ? "" : "s");
    juntaItem(nome, cand[j], true, d);
  }
  if (n == 0) { somErro(); poeBarra("Não conhecemos nenhuma cidade estrangeira ao alcance desta unidade.", ""); return; }
  strcpy(ajudaLista, "ESPAÇO: marchar até lá   ESC: cancelar");
  listaX0 = 60; listaW = 400; listaLinhas = 14;
  entraModo(M_LISTA);
  desenhaLista();
}
void marchaPara(int ci) {
  entraModo(M_MAPA);
  if (ativa < 0 || !U[ativa].viva) { desenhaTudo(); return; }
  Unidade & u = U[ativa];
  u.gx = CID[ci].x; u.gy = CID[ci].y; u.estado = EU_DESTINO;
  if (direcaoPara(u.x, u.y, u.gx, u.gy) >= 0) { u.estado = EU_NADA; u.gx = SEM; registaChegada(ativa, ci); }   // já está ao lado
  else segueDestino(ativa);
  if (!haChegadaPronta()) proximaUnidade();
  desenhaTudo();
  if (u.estado == EU_DESTINO) poeBarra("A caminho de", CID[ci].nome);
  continuaTurno();
}

// janela de confirmação: as duas forças lado a lado e a probabilidade de vencer
int ataqueUnid = -1, ataqueCid = -1, ataqueSel = 0;
void desenhaLadoAtaque(int x, int y, int i, const char * titulo, const char * linha2, int forca, RGB888 corForca) {
  char b[48];
  texto(x, y, titulo, C_CINZA);
  if (i >= 0) {
    int cor = corDe(U[i].civ);
    retangulo(x, y + 11, x + TILE_W - 3, y + 11 + TILE_H - 2, corCiv(cor));
    canvas.setPenColor(corCivE(cor)); canvas.drawRectangle(x, y + 11, x + TILE_W - 3, y + 11 + TILE_H - 2);
    canvas.drawBitmap(x - 1, y + 10, &BMP_UNI[U[i].tipo]);
    snprintf(b, sizeof(b), "%s%s", UNIDADE[U[i].tipo].nome, U[i].vet ? " (vet.)" : "");
    texto(x + TILE_W + 2, y + 12, b, C_BRANCO);
  } else texto(x, y + 16, "ninguém a defender!", C_VERDE);
  texto(x + (i >= 0 ? TILE_W + 2 : 0), y + 22, linha2, C_CINZA);
  snprintf(b, sizeof(b), "Força: %d", forca);
  texto(x, y + 34, b, corForca);
}
void desenhaAtaque() {
  const int i = ataqueUnid, ci = ataqueCid;
  const Cidade & k = CID[ci];
  char b[96], l2[48];
  const int x0 = 20, x1 = MAPA_PX_W - 20, y0 = 30, y1 = 152;
  caixa(x0, y0, x1, y1, C_PAINEL, C_MOLDURA);
  snprintf(b, sizeof(b), "Chegámos a %s (%s, tamanho %d)", k.nome, nomePovo(k.civ), k.tam);
  textoCentro(x0, x1, y0 + 4, b, C_AMARELO);
  int d = defensorEm(k.x, k.y, i);
  int fa = forcaAtaque(i), fd = d >= 0 ? forcaDefesa(d, i) : 0;
  snprintf(l2, sizeof(l2), "Ataq %d", UNIDADE[U[i].tipo].ataque);
  desenhaLadoAtaque(x0 + 10, y0 + 18, i, "Nós:", l2, fa, C_VERDE);
  if (d >= 0) {
    int l = snprintf(l2, sizeof(l2), "Def %d", UNIDADE[U[d].tipo].defesa);
    if (U[d].estado == EU_FORTIFICADA) l += snprintf(l2 + l, sizeof(l2) - l, ", fort.");
    if (temEdif(k, E_MURALHAS) || temMaravilha(k.civ, W_MURALHA)) snprintf(l2 + l, sizeof(l2) - l, ", muralha");
  }
  int lista[16], nd = unidadesEm(k.x, k.y, lista, 16);
  snprintf(b, sizeof(b), "Eles (%d na cidade):", nd);
  desenhaLadoAtaque(x0 + 200, y0 + 18, d, b, d >= 0 ? l2 : "", fd, C_VERMELHO);
  // probabilidade de vencer (o combate é decidido em proporção às forças)
  int pct = d < 0 ? 100 : fa * 100 / max(1, fa + fd);
  snprintf(b, sizeof(b), "Hipótese de vitória: %d%%", pct);
  textoCentro(x0, x1, y0 + 64, b, pct >= 60 ? C_VERDE : pct >= 40 ? C_AMARELO : C_VERMELHO);
  const int bx0 = x0 + 40, bx1 = x1 - 40, by = y0 + 75;
  retangulo(bx0, by, bx1, by + 5, C_VERMELHO);
  retangulo(bx0, by, bx0 + (bx1 - bx0) * pct / 100, by + 5, C_VERDE);
  canvas.setPenColor(C_PRETO); canvas.drawRectangle(bx0 - 1, by - 1, bx1 + 1, by + 6);
  if (k.civ > 0 && !emGuerra(jogador, k.civ)) {
    snprintf(b, sizeof(b), "Em paz com os %s: atacar declara guerra!", nomePovo(k.civ));
    textoCentro(x0, x1, y0 + 85, b, C_VERMELHO);
  }
  const char * op[2] = { d >= 0 ? "Atacar!" : "Ocupar!", "Esperar aqui" };
  for (int q = 0; q < 2; ++q) {
    int bx = x0 + 100 + q * 150;
    retangulo(bx - 6, y1 - 16, bx + 116, y1 - 5, q == ataqueSel ? C_MOLDURA : C_PAINEL);
    textoCentro(bx - 6, bx + 116, y1 - 15, op[q], q == ataqueSel ? C_AMARELO : C_BRANCO);
  }
}
void abreAtaque(int i, int ci) {
  ataqueUnid = i; ataqueCid = ci; ataqueSel = 0;
  ativa = i; ultimaAtiva = i;
  centraEm(U[i].x, U[i].y);
  entraModo(M_ATAQUE);
  desenhaTudo();
  desenhaAtaque();
}
void tentaMover(int d);
void atualizaAtaque() {
  bool mudou = false;
  if (menuEsq() || menuDir() || menuCima() || menuBaixo()) { ataqueSel ^= 1; mudou = true; }
  if (mudou) { somMenu(); desenhaAtaque(); }
  bool sim = false, nao = false;
  if (premiuLetra('S') || premiuLetra('A')) sim = true;
  if (premiuLetra('N') || premiuLetra('E') || carregouVolta()) nao = true;
  if (tempo > 10 && carregouFogo()) { if (ataqueSel == 0) sim = true; else nao = true; }
  if (!sim && !nao) return;
  somOk();
  entraModo(M_MAPA);
  int i = ataqueUnid, ci = ataqueCid;
  if (sim && U[i].viva && CID[ci].viva) {
    int d = direcaoPara(U[i].x, U[i].y, CID[ci].x, CID[ci].y);
    if (d >= 0) {
      if (CID[ci].civ > 0 && !emGuerra(jogador, CID[ci].civ)) declaraGuerra(jogador, CID[ci].civ);
      ativa = i;
      desenhaTudo();
      tentaMover(d);
      return;
    }
  }
  desenhaTudo();
  poeBarra("A unidade espera junto à cidade.", "");
}

// ---------------- descoberta: o que é a tecnologia e o que permite ----------------
int tecDescobertaAtual = 0, descobertaSabios = 0;
// descomprime a ilustração da tecnologia t (RLE) para um buffer em PSRAM e desenha-a com moldura
void desenhaIlustracao(int t, int x, int y) {
  static uint8_t * buf = nullptr;
  if (!buf) buf = (uint8_t *) reserva(ILUS_W * ILUS_H);
  if (!buf || t < 0 || t >= N_ILUS) return;
  int p = 0;
  for (uint32_t i = ilus_inicio[t]; i < ilus_inicio[t + 1] && p < ILUS_W * ILUS_H; i += 2) {
    int n = ilus_dados[i]; uint8_t c = ilus_dados[i + 1];
    while (n-- > 0 && p < ILUS_W * ILUS_H) buf[p++] = c;
  }
  Bitmap bmp(ILUS_W, ILUS_H, buf, PixelFormat::RGBA2222);
  retangulo(x - 3, y - 2, x + ILUS_W + 2, y + ILUS_H + 1, C_OURO);
  canvas.setPenColor(RGB888(85, 0, 0)); canvas.drawRectangle(x - 3, y - 2, x + ILUS_W + 2, y + ILUS_H + 1);
  canvas.drawBitmap(x, y, &bmp);
  canvas.waitCompletion(false);                 // o Bitmap é local: esperar que seja desenhado
}
void desenhaDescoberta() {
  const int t = tecDescobertaAtual;
  char b[120], ano[24];
  limpaEcra(C_PAINEL);
  // faixa de cima
  retangulo(1, 1, ECRA_W - 2, 34, RGB888(0, 0, 85));
  retangulo(1, 35, ECRA_W - 2, 36, C_OURO);
  textoCentro(0, ECRA_W - 1, 3, descobertaSabios ? "Os nossos sábios descobriram:" : "Uma nova tecnologia para o nosso povo:", C_CINZA);
  {
    char c[64]; int n = converte(TEC[t].nome, c, sizeof(c));
    int w = 0; for (int i = 0; i < n; ++i) w += 8;
    textoGrande((ECRA_W - w * 2) / 2 + 2, 14, TEC[t].nome, RGB888(85, 0, 0), 2, 2, &fonte);   // sombra
    textoGrande((ECRA_W - w * 2) / 2, 13, TEC[t].nome, C_AMARELO, 2, 2, &fonte);
  }
  textoAno(ano, turno);
  texto(8, 3, ano, C_CINZA);
  snprintf(b, sizeof(b), "%d tecnologias", CIV[jogador].nTecs);
  textoDireita(ECRA_W - 8, 3, b, C_CINZA);
  // a ilustração (à esquerda) e o texto da enciclopédia (à direita)
  desenhaIlustracao(t, 12, 42);
  int y = max(42 + textoQuebrado(12 + ILUS_W + 12, 43, 54, TEC_TEXTO[t], C_BRANCO) + 4, 42 + ILUS_H + 6);
  // o que passa a ser possível
  bool algo = false;
  int nu = 0;
  for (int p = 0; p < N_UNIDADES; ++p) if (UNIDADE[p].tec == t) ++nu;
  if (nu) {
    texto(12, y, "Novas unidades:", C_AMARELO); y += 10;
    int k = 0;
    for (int p = 0; p < N_UNIDADES; ++p) if (UNIDADE[p].tec == t) {
      int x = 12 + k * 206;
      if (x + 200 > ECRA_W) break;
      // a unidade desenhada num escudo com a nossa cor
      retangulo(x, y, x + TILE_W - 3, y + TILE_H - 2, corCiv(corDe(jogador)));
      canvas.setPenColor(corCivE(corDe(jogador))); canvas.drawRectangle(x, y, x + TILE_W - 3, y + TILE_H - 2);
      canvas.drawBitmap(x - 1, y - 1, &BMP_UNI[p]);
      texto(x + TILE_W + 2, y + 1, UNIDADE[p].nome, C_BRANCO);
      snprintf(b, sizeof(b), "Ataq %d Def %d Mov %d", UNIDADE[p].ataque, UNIDADE[p].defesa, UNIDADE[p].mov);
      texto(x + TILE_W + 2, y + 10, b, C_CINZA);
      ++k;
    }
    y += TILE_H + 3; algo = true;
  }
  for (int e = 0; e < N_EDIFICIOS_TOTAL; ++e) if (EDIFICIO[e].tec == t && y < 172) {
    snprintf(b, sizeof(b), "Edifício: %s", EDIFICIO[e].nome);
    texto(12, y, b, C_VERDE);
    { char c[80]; int n = converte(b, c, sizeof(c));            // o efeito vai a seguir ao nome, sem se sobrepor
      int xe = max(12 + 30 * 8, 12 + (n + 2) * 8);
      if (xe + 8 * (int) strlen(EDIF_EFEITO[e]) > ECRA_W - 8) { y += 9; xe = 12 + 4 * 8; }
      texto(xe, y, EDIF_EFEITO[e], C_CINZA); }
    y += 10; algo = true;
  }
  for (int w = 0; w < N_MARAVILHAS; ++w) if (MARAVILHA[w].tec == t && y < 172) {
    snprintf(b, sizeof(b), "Maravilha: %s", MARAVILHA[w].nome);
    texto(12, y, b, C_OURO);
    { char c[80]; int n = converte(b, c, sizeof(c));            // o efeito vai a seguir ao nome, sem se sobrepor
      int xe = max(12 + 30 * 8, 12 + (n + 2) * 8);
      if (xe + 8 * (int) strlen(MARAVILHA[w].efeito) > ECRA_W - 8) { y += 9; xe = 12 + 4 * 8; }
      texto(xe, y, MARAVILHA[w].efeito, C_CINZA); }
    y += 10; algo = true;
  }
  for (int g = G_DESPOTISMO + 1; g < N_GOVERNOS; ++g) if (GOVERNO[g].tec == t && y < 172) {
    snprintf(b, sizeof(b), "Governo: %s (menu do jogo, Revolução)", GOVERNO[g].nome);
    texto(12, y, b, C_CEU);
    y += 10; algo = true;
  }
  if (!algo && y < 172) { texto(12, y, "Abre caminho a novas descobertas.", C_CINZA); y += 10; }
  // o que se pode estudar a seguir graças a ela
  int p = snprintf(b, sizeof(b), "Leva a:");
  int n = 0;
  for (int q = 0; q < N_TECS; ++q) if (TEC[q].pre1 == t || TEC[q].pre2 == t) {
    if (p > 70) break;
    p += snprintf(b + p, sizeof(b) - p, "%s %s", n ? "," : "", TEC[q].nome);
    ++n;
  }
  if (n) texto(12, 177, b, C_CINZA);
  textoDireita(ECRA_W - 8, 188, "ESPAÇO: continuar", C_CINZA);
  moldura();
}
void abreDescoberta(int t, int sabios) {
  tecDescobertaAtual = t; descobertaSabios = sabios;
  entraModo(M_DESCOBERTA);
  somTri(523, 200, 60); somTri(659, 200, 60); somTri(784, 300, 80);
  desenhaDescoberta();
}
void atualizaDescoberta() {
  if (tempo > 10 && (carregouFogo() || carregouVolta())) {
    somOk();
    entraModo(M_MAPA);
    desenhaTudo();
    continuaTurno();
  }
}

// o que falta mostrar no início do turno: mensagens, propostas, pesquisa, produção
void continuaTurno() {
  if (nMsg > 0) { mostraMensagens(); return; }
  if (nCenas > 0) {
    CenaPendente c = cenasPendentes[0];
    --nCenas;
    for (int k = 0; k < nCenas; ++k) cenasPendentes[k] = cenasPendentes[k + 1];
    abreCena(c.tipo, c.a, c.c, c.b);
    return;
  }
  while (encontrosPendentes) {
    int c = 0;
    while (!(encontrosPendentes >> c & 1)) ++c;
    encontrosPendentes &= ~(1 << c);
    if (c > 0 && c != jogador && CIV[c].viva) { abreEncontro(c); return; }
  }
  if (nTecsNovas > 0) {
    int t = tecsNovas[0], sab = tecsNovasSabios[0];
    --nTecsNovas;
    for (int k = 0; k < nTecsNovas; ++k) { tecsNovas[k] = tecsNovas[k + 1]; tecsNovasSabios[k] = tecsNovasSabios[k + 1]; }
    if (sab) abreCena(CN_DESCOBERTA, t, 0, 0);      // os sábios no gabinete; a seguir, a janela da descoberta
    else abreDescoberta(t, sab);
    return;
  }
  for (int k = 0; k < nChegadas; ++k) {
    int i = chegouUnid[k], ci = chegouCid[k];
    // já não serve: a unidade morreu, saiu dali, ou a cidade mudou de dono
    if (!U[i].viva || U[i].civ != jogador || !CID[ci].viva || CID[ci].civ == jogador || direcaoPara(U[i].x, U[i].y, CID[ci].x, CID[ci].y) < 0) { tiraChegada(k); --k; continue; }
    if (U[i].mov == 0) continue;                       // ataca no próximo turno
    tiraChegada(k);
    abreAtaque(i, ci);
    return;
  }
  if (proposta != PR_NADA) {
    TipoProposta p = proposta;
    proposta = PR_NADA;
    const char * lider = POVO[CIV[propostaCiv].povo].lider;
    switch (p) {
      case PR_PAZ: pergunta(Q_PAZ, "%s, dos %s, propõe paz entre os nossos povos. Aceitas?", lider, nomePovo(propostaCiv)); return;
      case PR_TRIBUTO: pergunta(Q_TRIBUTO, "%s, dos %s, exige %d$ de tributo. Se recusares, é a guerra. Pagas?", lider, nomePovo(propostaCiv), propostaOuro); return;
      case PR_GUERRA: mensagem("%s, dos %s: \"Os vossos dias estão contados!\" Os %s declararam-nos guerra.", lider, nomePovo(propostaCiv), nomePovo(propostaCiv)); mostraMensagens(); return;
      default: break;
    }
  }
  if (CIV[jogador].pesquisa == SEM_TEC && contaPesquisaveis(jogador) > 0 && CIV[jogador].anarquia == 0) { desenhaTudo(); abrePesquisa(); return; }
  while (nPedeProducao > 0) {
    int ci = pedeProducao[--nPedeProducao];
    if (!CID[ci].viva || CID[ci].civ != jogador) continue;
    char t[64]; snprintf(t, sizeof(t), "%s: o que fazer agora?", CID[ci].nome);
    centraEm(CID[ci].x, CID[ci].y);
    desenhaTudo();
    abreProducao(ci, t);
    return;
  }
  entraModo(M_MAPA);
  if (ativa < 0 || !unidadeEsperaOrdens(ativa)) proximaUnidade();
  desenhaTudo();
}

// ---------------- respostas ----------------
void respostaPergunta(bool sim) {
  IdPergunta q = idPergunta;
  idPergunta = Q_NADA;
  switch (q) {
    case Q_GUERRA:
      entraModo(M_MAPA);
      if (sim && ativa >= 0 && U[ativa].viva) {
        int d = perguntaDir;
        moveUnidade(ativa, d, true);
        if (!U[ativa].viva || U[ativa].mov == 0) proximaUnidade();
      }
      mostraDepoisDeAcao();
      return;
    case Q_PAZ:
      if (sim) { fazPaz(jogador, propostaCiv); mensagem("Paz assinada com os %s.", nomePovo(propostaCiv)); }
      break;
    case Q_TRIBUTO:
      if (sim) { CIV[jogador].ouro -= propostaOuro; CIV[propostaCiv].ouro += propostaOuro; }
      else declaraGuerra(propostaCiv, jogador);
      break;
    case Q_SAIR:
      if (sim) { void entraTitulo(); entraTitulo(); return; }
      entraModo(M_MAPA); desenhaTudo(); return;
    case Q_NAVE:
      if (sim) {
        CIV[jogador].naveChega = turno + max(8, 20 - CIV[jogador].nave[1] * 2);
        mensagem("A nave partiu para Alfa do Centauro! Chega no turno %d, se a capital não cair.", CIV[jogador].naveChega);
      }
      break;
    default: break;
  }
  continuaTurno();
}

void atualizaPergunta() {
  if (menuEsq() || menuDir()) { perguntaSel ^= 1; somMenu(); desenhaPergunta(); }
  if (premiuLetra('S')) { perguntaSel = 0; respostaPergunta(true); return; }
  if (premiuLetra('N')) { perguntaSel = 1; respostaPergunta(false); return; }
  if (tempo > 6 && carregouFogo()) { somOk(); respostaPergunta(perguntaSel == 0); return; }
  if (carregouVolta()) { respostaPergunta(false); return; }
}

// escolha numa lista
void entraTitulo();
void desenhaNovoJogo();
uint8_t passoNovo = 0;

// ---------------- listas de gravações ----------------
void juntaGravacao(int k, bool vazias) {
  Cabecalho c;
  char t[64], d[16], n[12];
  if (k == GRAV_AUTO) strcpy(n, "Auto:"); else if (k == GRAV_ANTIGA) strcpy(n, "Antiga:"); else snprintf(n, sizeof(n), "%d.", (k + 1) % 100);
  if (leCabecalho(k, c)) {
    char desc[48]; descreveGravacao(c, desc, sizeof(desc), d, sizeof(d));
    snprintf(t, sizeof(t), "%s %s", n, desc);
    juntaItem(t, k, true, d);
  } else if (vazias) {
    snprintf(t, sizeof(t), "%s (vazia)", n);
    juntaItem(t, k, true);
  }
}
void abreGravar() {
  if (!montaSD()) { poeBarra("Não foi possível gravar: o cartão SD não responde.", ""); somErro(); return; }
  modoAntesLista = M_MAPA;
  limpaLista("Gravar o jogo: em que posição?", L_GRAVAR);
  for (int k = 0; k < N_GRAVACOES; ++k) juntaGravacao(k, true);
  if (ultimaGravacao >= 0) itemSel = ultimaGravacao;
  strcpy(ajudaLista, "ESPAÇO: gravar aqui   ESC: cancelar");
  listaX0 = 80; listaW = 400; listaLinhas = 12;
  entraModo(M_LISTA);
  desenhaLista();
}
void abreCarregar() {
  modoAntesLista = M_TITULO;
  limpaLista("Carregar um jogo gravado", L_CARREGAR);
  for (int k = 0; k < N_GRAVACOES; ++k) juntaGravacao(k, false);
  juntaGravacao(GRAV_AUTO, false);
  juntaGravacao(GRAV_ANTIGA, false);
  if (nItens == 0) { entraTitulo(); return; }
  strcpy(ajudaLista, "ESPAÇO: carregar   ESC: voltar");
  listaX0 = 120; listaW = 400; listaLinhas = 13;
  limpaEcra(C_PRETO);
  entraModo(M_LISTA);
  desenhaLista();
}

// ---------------- criar o mundo (como no original: terra, clima, temperatura, idade; e mais o tamanho,
// as ilhas e os recursos) com uma pré-visualização do mapa ----------------
const int N_LINHAS_MUNDO = 10;     // 7 opções + outro mundo + ao acaso + começar
int linhaMundo = 9;
const char * const NOME_TAM[4]   = { "Pequeno (64x40)", "Normal (80x50)", "Grande (96x60)", "Enorme (112x70)" };
const char * const NOME_TERRA[3] = { "Pequena", "Normal", "Grande" };
const char * const NOME_ILHAS[3] = { "Poucas: continentes", "Normal", "Muitas: arquipélagos" };
const char * const NOME_CLIMA[3] = { "Árido (desertos)", "Normal", "Húmido (selvas)" };
const char * const NOME_TEMP[3]  = { "Fria", "Temperada", "Quente" };
const char * const NOME_IDADE[3] = { "3 mil milhões", "4 mil milhões", "5 mil milhões" };
const char * const NOME_REC[3]   = { "Poucos", "Normais", "Muitos" };
uint8_t * campoMundo(int l) {
  switch (l) {
    case 0: return &mundo.tamanho; case 1: return &mundo.terra; case 2: return &mundo.ilhas; case 3: return &mundo.clima;
    case 4: return &mundo.temperatura; case 5: return &mundo.idade; case 6: return &mundo.recursos;
  }
  return nullptr;
}
int nValoresMundo(int l) { return l == 0 ? 4 : 3; }
const char * textoMundo(int l) {
  switch (l) {
    case 0: return NOME_TAM[mundo.tamanho % 4]; case 1: return NOME_TERRA[mundo.terra % 3]; case 2: return NOME_ILHAS[mundo.ilhas % 3];
    case 3: return NOME_CLIMA[mundo.clima % 3]; case 4: return NOME_TEMP[mundo.temperatura % 3]; case 5: return NOME_IDADE[mundo.idade % 3];
    case 6: return NOME_REC[mundo.recursos % 3];
  }
  return "";
}
void geraPrevisao() {
  poeTamanhoMapa(TAM_W[mundo.tamanho % 4], TAM_H[mundo.tamanho % 4]);
  geraMundo(sementeEscolhida);
}
void desenhaOpcoesMundo() {
  static const char * const ROT[N_LINHAS_MUNDO] = { "Tamanho do mapa", "Massa de terra", "Ilhas", "Clima", "Temperatura",
                                                     "Idade do mundo", "Recursos", "Outro mundo", "Tudo ao acaso", "Começar!" };
  retangulo(4, 16, 300, 196, C_PAINEL);
  for (int l = 0; l < N_LINHAS_MUNDO; ++l) {
    int y = 22 + l * 16 + (l >= 7 ? 6 : 0);
    bool sel = l == linhaMundo;
    if (sel) retangulo(8, y - 2, 296, y + 9, C_MOLDURA);
    texto(14, y, ROT[l], sel ? C_AMARELO : (l >= 7 ? C_CEU : C_BRANCO));
    if (l < 7) {
      char b[40]; snprintf(b, sizeof(b), "< %s >", textoMundo(l));
      textoDireita(292, y, b, sel ? C_AMARELO : C_CINZA);
    }
  }
}
void desenhaPrevisaoMundo() {
  // o mapa todo na área da direita, com a proporção certa
  const int X0 = 312, Y0 = 22, W = 316, H = 118;
  retangulo(X0 - 2, Y0 - 2, X0 + W + 1, Y0 + H + 1, C_PRETO);
  int w = W, h = W * MAPA_H * 10 / (MAPA_W * 24);      // píxeis altos: 1 linha = 2,4 colunas
  if (h > H) { h = H; w = H * MAPA_W * 24 / (MAPA_H * 10); }
  desenhaMundoPequeno(X0 + (W - w) / 2, Y0 + (H - h) / 2, w, h, false);
  // contas do terreno
  int n[N_TERRENOS] = {}, terra = 0;
  for (int i = 0; i < MAPA_N; ++i) { int t = mapaTer[i] & 15; ++n[t]; if (t != T_OCEANO) ++terra; }
  auto pct = [&](int t) { return terra ? n[t] * 100 / terra : 0; };
  char b[96];
  retangulo(X0 - 2, 146, ECRA_W - 5, 196, C_PAINEL);
  snprintf(b, sizeof(b), "Terra %d%% do mapa   Massas de terra: %d", terra * 100 / MAPA_N, marcaContinentes());
  texto(X0, 148, b, C_BRANCO);
  snprintf(b, sizeof(b), "Prado %d%%  Planície %d%%  Deserto %d%%", pct(T_PRADO) + pct(T_RIO), pct(T_PLANICIE), pct(T_DESERTO));
  texto(X0, 160, b, C_VERDE);
  snprintf(b, sizeof(b), "Floresta %d%%  Selva %d%%  Pântano %d%%", pct(T_FLORESTA), pct(T_SELVA), pct(T_PANTANO));
  texto(X0, 170, b, C_VERDE);
  snprintf(b, sizeof(b), "Colinas %d%%  Montanhas %d%%  Frio %d%%", pct(T_COLINAS), pct(T_MONTANHAS), pct(T_TUNDRA) + pct(T_ARTICO));
  texto(X0, 180, b, C_CINZA);
}
void desenhaMundo() {
  limpaEcra(C_PAINEL);
  textoCentro(0, ECRA_W - 1, 3, "Criar o mundo", C_AMARELO);
  canvas.setPenColor(C_MOLDURA); canvas.drawLine(1, 13, ECRA_W - 2, 13);
  desenhaOpcoesMundo();
  desenhaPrevisaoMundo();
  textoDireita(ECRA_W - 8, 3, "Setas: mudar   ENTER: começar", C_CINZA);
  moldura();
}
void abreMundo() {
  sementeEscolhida = (millis() * 2654435761UL + 12345) | 1;
  linhaMundo = 9;
  entraModo(M_MUNDO);
  limpaEcra(C_PAINEL);
  textoCentro(0, ECRA_W - 1, 90, "A criar o mundo...", C_AMARELO);
  canvas.waitCompletion(false);
  geraPrevisao();
  desenhaMundo();
}
void abrePovos() {
  limpaLista("Escolhe o teu povo", L_POVO);
  for (int p = 0; p < N_POVOS; ++p) juntaItem(POVO[p].nome, p, true, POVO[p].lider);
  listaX0 = 140; listaW = 360; listaLinhas = 15;
  limpaEcra(C_PRETO);
  entraModo(M_LISTA);
  modoAntesLista = M_TITULO;
  desenhaLista();
}
void atualizaMundo() {
  if (menuCima())  { linhaMundo = (linhaMundo + N_LINHAS_MUNDO - 1) % N_LINHAS_MUNDO; somMenu(); desenhaOpcoesMundo(); moldura(); }
  if (menuBaixo()) { linhaMundo = (linhaMundo + 1) % N_LINHAS_MUNDO; somMenu(); desenhaOpcoesMundo(); moldura(); }
  int d = menuEsq() ? -1 : menuDir() ? 1 : 0;
  uint8_t * c = campoMundo(linhaMundo);
  bool refaz = false;
  if (d && c) { int n = nValoresMundo(linhaMundo); *c = (*c + n + d) % n; refaz = true; }
  if (carregouVolta()) { entraTitulo(); return; }
  if (carregouFogo()) {
    if (linhaMundo == 7) { sementeEscolhida = aleat() | 1; refaz = true; }              // outro mundo, as mesmas opções
    else if (linhaMundo == 8) {                                                          // tudo ao acaso
      mundo.tamanho = aleat(4); mundo.terra = aleat(3); mundo.ilhas = aleat(3); mundo.clima = aleat(3);
      mundo.temperatura = aleat(3); mundo.idade = aleat(3); mundo.recursos = aleat(3);
      sementeEscolhida = aleat() | 1; refaz = true;
    } else { somOk(); abrePovos(); return; }                                              // começar (ENTER em qualquer opção)
  }
  if (refaz) {
    somMenu();
    desenhaOpcoesMundo();
    textoCentro(310, ECRA_W - 4, 74, "a criar...", C_AMARELO);
    canvas.waitCompletion(false);
    uint32_t r = rngEstado;
    geraPrevisao();
    rngEstado = r ^ millis();
    desenhaPrevisaoMundo();
    moldura();
  }
}

// ---------------- o nome do jogador (como no original) ----------------
char nomeEdit[24];
int nomeCidadeAlvo = -1;            // M_NOME: -1 = nome do jogador; >= 0 = nome da cidade acabada de fundar
int caixaNomeX = 180, caixaNomeY = 132;
void desenhaCaixaNome() {
  caixa(caixaNomeX, caixaNomeY, caixaNomeX + 280, caixaNomeY + 16, C_PRETO, C_MOLDURA);
  retangulo(caixaNomeX + 4, caixaNomeY + 3, caixaNomeX + 276, caixaNomeY + 13, C_PRETO);
  texto(caixaNomeX + 10, caixaNomeY + 4, nomeEdit, C_AMARELO);
  if ((frame / 15) & 1) { int w = letras(nomeEdit) * 8; retangulo(caixaNomeX + 10 + w, caixaNomeY + 11, caixaNomeX + 16 + w, caixaNomeY + 12, C_AMARELO); }
}
// ---- nome de uma cidade nova: sugere-se um nome, que se pode mudar na hora ----
void desenhaNomeCidade() {
  desenhaTudo();
  caixa(100, 56, 540, 132, C_PAINEL, C_OURO);
  textoCentro(100, 540, 62, "Fundámos uma nova cidade!", C_AMARELO);
  char b[80], ano[24]; textoAno(ano, turno);
  snprintf(b, sizeof(b), "Ano %s: que nome lhe damos?", ano);
  textoCentro(100, 540, 76, b, C_BRANCO);
  caixaNomeX = 180; caixaNomeY = 92;
  desenhaCaixaNome();
  textoCentro(100, 540, 116, "Escreve o nome (até 15 letras).  ENTER: fundar", C_CINZA);
  moldura();
}
void abreNomeCidade(int ci) {
  nomeCidadeAlvo = ci;
  strncpy(nomeEdit, CID[ci].nome, sizeof(nomeEdit) - 1); nomeEdit[sizeof(nomeEdit) - 1] = 0;
  entraModo(M_NOME);
  desenhaNomeCidade();
}
void acabaNomeCidade() {
  int ci = nomeCidadeAlvo;
  nomeCidadeAlvo = -1;
  caixaNomeX = 180; caixaNomeY = 132;
  // tira os espaços das pontas; vazio = fica o nome sugerido
  char * p = nomeEdit; while (*p == ' ') ++p;
  int n = strlen(p); while (n > 0 && p[n - 1] == ' ') p[--n] = 0;
  if (n > 0 && ci >= 0) {
    char novo[sizeof(CID[ci].nome)];
    snprintf(novo, sizeof(novo), "%s", p);
    strcpy(CID[ci].nome, novo);
  }
  if (ci >= 0 && CID[ci].viva) abreProducao(ci, "Nova cidade! O que vamos fazer?");
  else { entraModo(M_MAPA); desenhaTudo(); }
}
void desenhaNome() {
  limpaEcra(C_PAINEL);
  char b[96];
  textoCentro(0, ECRA_W - 1, 4, "Um novo povo entra na história", C_AMARELO);
  // o retrato do líder do povo escolhido
  retangulo(238, 18, 401, 104, C_OURO);
  retangulo(240, 20, 399, 103, RGB888(85, 0, 0));
  desenhaArte(240, 20, dadosRetrato(povoEscolhido % N_RETRATOS), RETRATO_W, RETRATO_H);
  snprintf(b, sizeof(b), "Como te chamas, líder dos %s?", POVO[povoEscolhido].nome);
  textoCentro(0, ECRA_W - 1, 116, b, C_BRANCO);
  desenhaCaixaNome();
  textoCentro(0, ECRA_W - 1, 160, "Escreve o nome e carrega em ENTER (até 23 letras).", C_CINZA);
  textoCentro(0, ECRA_W - 1, 172, "O nome aparece nas gravações, como no original.", C_CINZA);
  moldura();
}
void abreNome() {
  nomeCidadeAlvo = -1; caixaNomeX = 180; caixaNomeY = 132;
  strncpy(nomeEdit, POVO[povoEscolhido].lider, sizeof(nomeEdit) - 1); nomeEdit[sizeof(nomeEdit) - 1] = 0;
  // não cortar uma letra acentuada a meio
  int n = strlen(nomeEdit); while (n > 0 && ((uint8_t)nomeEdit[n - 1] & 0xC0) == 0xC0) nomeEdit[--n] = 0;
  entraModo(M_NOME);
  desenhaNome();
}
void comecaJogoNovo() {
  strncpy(nomeJogador, nomeEdit, sizeof(nomeJogador) - 1); nomeJogador[sizeof(nomeJogador) - 1] = 0;
  if (!nomeJogador[0]) strncpy(nomeJogador, POVO[povoEscolhido].lider, sizeof(nomeJogador) - 1);
  ultimaGravacao = -1;
  limpaEcra(C_PRETO);
  textoCentro(0, ECRA_W - 1, 90, "A criar o mundo...", C_AMARELO);
  canvas.waitCompletion(false);
  novoJogo();
  char b[300];
  snprintf(b, sizeof(b), "No ano 4000 a.C., o povo dos %s, guiado por %s, deixa a vida de nómadas e procura um lugar para a primeira cidade. "
           "Usa os colonos para fundar cidades (B), faz crescer o império, descobre tecnologias e constrói maravilhas. "
           "Conquista o mundo, ou chega primeiro às estrelas.",
           POVO[povoEscolhido].nome, nomeJogador);
  mensagem("%s", b);
  TOCA(MUS_TITULO, BAI_TITULO, 120, false);
  comecaTurnoJogador();
}
void atualizaNome() {
  bool mudou = false;
  int n = strlen(nomeEdit);
  for (int i = 0; i < nTeclas; ++i) {
    VirtualKey vk = filaTeclas[i];
    if (vk == VK_RETURN || vk == VK_KP_ENTER) { somOk(); if (nomeCidadeAlvo >= 0) acabaNomeCidade(); else comecaJogoNovo(); return; }
    if (vk == VK_ESCAPE) {
      if (nomeCidadeAlvo >= 0) { strncpy(nomeEdit, CID[nomeCidadeAlvo].nome, sizeof(nomeEdit) - 1); acabaNomeCidade(); }   // fica o sugerido
      else entraTitulo();
      return;
    }
    if (vk == VK_BACKSPACE || vk == VK_DELETE) {
      while (n > 0 && ((uint8_t)nomeEdit[n - 1] & 0xC0) == 0x80) --n;      // continuação de UTF-8
      if (n > 0) --n;
      nomeEdit[n] = 0; mudou = true;
      continue;
    }
    int a = kb ? kb->virtualKeyToASCII(vk) : -1;
    const int maxN = nomeCidadeAlvo >= 0 ? (int)sizeof(CID[0].nome) - 1 : (int)sizeof(nomeEdit) - 1;
    if (n == 0 && a >= 'a' && a <= 'z') a -= 32;              // a primeira letra vai em maiúscula
    if (a >= 32 && a < 127 && n < maxN) { nomeEdit[n++] = (char)a; nomeEdit[n] = 0; mudou = true; }
  }
  if (joyFogo()) { somOk(); if (nomeCidadeAlvo >= 0) acabaNomeCidade(); else comecaJogoNovo(); return; }
  if (mudou) somMenu();
  if (mudou || tempo % 15 == 0) desenhaCaixaNome();
}

void escolhaLista(int s) {
  int v = s >= 0 ? itens[s].valor : -1;
  switch (idLista) {
    case L_TITULO:
      if (s < 0) { entraTitulo(); return; }
      if (v == 1) { abreCarregar(); return; }
      limpaLista("Nível de dificuldade", L_DIFICULDADE);
      for (int d = 0; d < N_DIFICULDADES; ++d) juntaItem(DIFICULDADE[d], d);
      itemSel = 1;
      listaX0 = 200; listaW = 240; listaLinhas = 6;
      desenhaLista();
      return;
    case L_DIFICULDADE:
      if (s < 0) { entraTitulo(); return; }
      dificuldade = v;
      limpaLista("Quantos povos no mundo?", L_NCIVS);
      for (int n = 3; n <= 7; ++n) { char t[20]; sprintf(t, "%d povos", n); juntaItem(t, n); }
      itemSel = 4;
      listaX0 = 200; listaW = 240; listaLinhas = 6;
      desenhaLista();
      return;
    case L_NCIVS:
      if (s < 0) { entraTitulo(); return; }
      nCivsJogo = v;
      abreMundo();
      return;
    case L_POVO: {
      if (s < 0) { entraTitulo(); return; }
      povoEscolhido = v;
      abreNome();
      return;
    }
    case L_GRAVAR:
      entraModo(M_MAPA); desenhaTudo();
      if (s >= 0) poeBarra(gravaJogo(v) ? "Jogo gravado no cartão SD." : "Não foi possível gravar: o cartão SD não responde.", "");
      return;
    case L_CARREGAR:
      if (s < 0) { entraTitulo(); return; }
      if (carregaJogo(v)) {
        ativa = -1; ultimaAtiva = -1;
        nChegadas = 0; nTecsNovas = 0; encontrosPendentes = 0; nMsg = 0; nPedeProducao = 0; nCenas = 0;
        noTitulo = false; { int n = CIV[jogador].nTecs; epocaMusical = n < 15 ? 0 : n < 35 ? 1 : n < 55 ? 2 : 3; }
        proximaUnidade();
        if (ativa >= 0) centraEm(U[ativa].x, U[ativa].y);
        else if (CIV[jogador].capital != SEM) centraEm(CID[CIV[jogador].capital].x, CID[CIV[jogador].capital].y);
        TOCA(MUS_TITULO, BAI_TITULO, 120, false);
        entraModo(M_MAPA); desenhaTudo(); poeBarra("", ""); continuaTurno(); return;
      }
      entraTitulo(); return;
    case L_PRODUCAO:
      if (s >= 0 && cidadeAtual >= 0) {
        Cidade & k = CID[cidadeAtual];
        // mudar entre unidades, edifícios e maravilhas custa metade do que já estava feito (como no original)
        int antes = k.prod < PROD_EDIF ? 0 : k.prod < PROD_MARAV ? 1 : 2;
        int depois = v < PROD_EDIF ? 0 : v < PROD_MARAV ? 1 : 2;
        if (v == PROD_AUTO) {
          k.fundada |= CID_AUTO;
          int velho = k.prod;
          escolheProducaoIA(cidadeAtual);
          int d2 = k.prod < PROD_EDIF ? 0 : k.prod < PROD_MARAV ? 1 : 2;
          if (antes != d2 && k.prod != velho && k.escudos > 0) k.escudos /= 2;
        } else {
          k.fundada &= ~CID_AUTO;
          if (antes != depois && k.escudos > 0) k.escudos /= 2;
          k.prod = v;
        }
      }
      if (modoAntesLista == M_CIDADE) { entraModo(M_CIDADE); desenhaCidade(); return; }
      entraModo(M_MAPA); desenhaTudo(); continuaTurno(); return;
    case L_PESQUISA:
      if (s >= 0) CIV[jogador].pesquisa = v;
      else if (contaPesquisaveis(jogador) > 0) { abrePesquisa(); return; }   // tem de escolher
      continuaTurno(); return;
    case L_ORDENS:
      if (s < 0) { entraModo(M_MAPA); desenhaTudo(); return; }
      executaOrdem(v); return;
    case L_MENU:
      entraModo(M_MAPA);
      if (s < 0 || v == MN_VOLTAR) { desenhaTudo(); return; }
      switch (v) {
        case MN_TAXAS: taxaSel = 0; desenhaTudo(); entraModo(M_TAXAS); desenhaTaxas(); return;
        case MN_REVOLUCAO: abreGoverno(); return;
        case MN_DIPLOMACIA: abreDiplomacia(); return;
        case MN_CIDADES: abreListaCidades(); return;
        case MN_DORMIR: abreDormir(); return;
        case MN_CIENCIA: case MN_POVOS: case MN_MARAVILHAS: case MN_PONTOS: abreRelatorio(v); return;
        case MN_NAVE: desenhaTudo(); pergunta(Q_NAVE, "Lançar a nave com %d estruturas, %d componentes e %d módulos?", CIV[jogador].nave[0], CIV[jogador].nave[1], CIV[jogador].nave[2]); return;
        case MN_GRAVAR: desenhaTudo(); abreGravar(); return;
        case MN_MUSICA: musicaPermitida = !musicaPermitida; desenhaTudo(); return;
        case MN_INIMIGOS:
          verInimigos = !verInimigos;
          abreMenu();
          for (int k = 0; k < nItens; ++k) if (itens[k].valor == MN_INIMIGOS) itemSel = k;   // o cursor fica na opção
          desenhaLista();
          return;
        case MN_SAIR: desenhaTudo(); pergunta(Q_SAIR, "Sair para o título? (o que não estiver gravado perde-se)"); return;
      }
      desenhaTudo(); return;
    case L_GOVERNO:
      if (s >= 0) {
        mudaGoverno(jogador, v);
        ajustaTaxas(jogador);
        if (CIV[jogador].anarquia) mensagem("Revolução! O país está em anarquia durante 2 turnos.");
        else mensagem("O governo passa a ser: %s.", GOVERNO[v].nome);
      }
      entraModo(M_MAPA); desenhaTudo(); continuaTurno(); return;
    case L_IR_CIDADE:
      if (s < 0) {      // volta a escolher no mapa
        entraModo(M_DESTINO); desenhaTudo();
        poeBarra("Ir para: escolhe o destino com as setas e ESPAÇO.", "C: lista das nossas cidades.  ESC: cancelar.");
        return;
      }
      modoVer = false;
      entraModo(M_MAPA);
      if (ativa >= 0 && U[ativa].viva) {
        Unidade & u = U[ativa];
        u.gx = CID[v].x; u.gy = CID[v].y; u.estado = EU_DESTINO;
        segueDestino(ativa);
        proximaUnidade();
      }
      poeBarra("", "");
      if (nMsg) { desenhaTudo(); mostraMensagens(); return; }
      desenhaTudo();
      return;
    case L_DORMIR:
      if (s >= 0) { somOk(); acordaDormir(v); return; }
      entraModo(M_MAPA); desenhaTudo(); return;
    case L_ALVOS:
      if (s >= 0) { marchaPara(v); return; }
      entraModo(M_MAPA); desenhaTudo(); return;
    case L_CIDADES:
      if (s >= 0) { centraEm(CID[v].x, CID[v].y); abreCidade(v); return; }
      entraModo(M_MAPA); desenhaTudo(); return;
    case L_DIPLOMATA:
      entraModo(M_MAPA);
      if (s >= 0 && v != 4) acaoDiplomata(v);
      proximaUnidade();
      mostraDepoisDeAcao(); return;
    case L_CARAVANA:
      entraModo(M_MAPA);
      if (s >= 0 && v != 3) acaoCaravana(v);
      if (ativa >= 0 && (!U[ativa].viva || U[ativa].mov == 0)) proximaUnidade();
      mostraDepoisDeAcao(); return;
    case L_OPCOES_CIDADE:
      entraModo(M_CIDADE);
      switch (v) {
        case 0: abreProducao(cidadeAtual, CID[cidadeAtual].nome); return;
        case 1: compraNaCidade(); desenhaCidade(); return;
        case 2: abreVender(); return;
        case 3: { Cidade & k = CID[cidadeAtual]; for (int q = 0; q < 3; ++q) if (k.especialistas[q]) { --k.especialistas[q]; ++k.especialistas[(q + 1) % 3]; break; } desenhaCidade(); return; }
        case 5: abreUnidadesCidade(); return;
        case 4: case -1: entraModo(M_MAPA); desenhaTudo(); return;
      }
      desenhaCidade(); return;
    case L_UNID_CIDADE:
      if (s < 0) { entraModo(M_CIDADE); desenhaCidade(); return; }
      if (v == ACORDAR_TODAS) {
        const Cidade & k = CID[cidadeAtual];
        int lista[16], n = unidadesEm(k.x, k.y, lista, 16);
        for (int j = 0; j < n; ++j) if (U[lista[j]].civ == jogador) acordaUnidade(lista[j]);
        somOk();
        entraModo(M_CIDADE); desenhaCidade();
        abreUnidadesCidade();
        return;
      }
      acordaUnidade(v);
      somOk();
      if (U[v].mov > 0) {
        // fica já como unidade ativa, pronta para sair da cidade
        ativa = v; ultimaAtiva = v;
        entraModo(M_MAPA);
        centraEm(U[v].x, U[v].y);
        desenhaTudo();
        poeBarra("Unidade acordada: move-a com as setas.", "");
        return;
      }
      // sem movimento este turno: fica acordada e é chamada no próximo turno
      entraModo(M_CIDADE); desenhaCidade();
      abreUnidadesCidade(v);
      return;
    case L_VENDER:
      if (s >= 0) {
        Cidade & k = CID[cidadeAtual];
        k.edif &= ~(1UL << v);
        CIV[k.civ].ouro += EDIFICIO[v].custo;
        k.comprou = 1;
        somOk();
      }
      entraModo(M_CIDADE); desenhaCidade(); return;
    case L_DIPLOMACIA:
      if (s < 0) { entraModo(M_MAPA); desenhaTudo(); return; }
      civDiplomacia = v;
      modoAntesLista = M_MAPA;
      { char t[64]; snprintf(t, sizeof(t), "%s, dos %s", POVO[CIV[v].povo].lider, nomePovo(v)); limpaLista(t, L_DIPLO_ACAO); }
      if (CIV[jogador].guerra >> v & 1) juntaItem("Propor a paz", 0);
      else juntaItem("Declarar guerra", 1);
      juntaItem("Oferecer 50$ de presente", 2, CIV[jogador].ouro >= 50);
      juntaItem("Voltar", 3);
      listaX0 = 160; listaW = 300; listaLinhas = 5;
      desenhaLista();
      return;
    case L_DIPLO_ACAO: {
      int c = civDiplomacia;
      entraModo(M_MAPA);
      switch (v) {
        case 0: {
          // a IA aceita a paz se estiver a perder ou se não for muito agressiva
          int fa = forcaMilitar(c), fj = forcaMilitar(jogador);
          bool aceita = fa < fj || POVO[CIV[c].povo].agressivo == 0 || aleat(3) == 0;
          if (aceita) { fazPaz(jogador, c); mensagem("%s aceita a paz.", POVO[CIV[c].povo].lider); }
          else mensagem("%s recusa: \"Só haverá paz quando nós a quisermos.\"", POVO[CIV[c].povo].lider);
          break;
        }
        case 1: declaraGuerra(jogador, c); break;
        case 2: CIV[jogador].ouro -= 50; CIV[c].ouro += 50; if (CIV[c].atitude > 0) --CIV[c].atitude; mensagem("Os %s agradecem o presente.", nomePovo(c)); break;
      }
      mostraDepoisDeAcao();
      return;
    }
  }
}

void atualizaListaModo() {
  if (idLista == L_PRODUCAO && premiuLetra('A')) { somOk(); escolhaLista(0); return; }   // Auto
  int s = atualizaLista();
  if (s == -1) return;
  escolhaLista(s == -2 ? -1 : s);
}

// ---------------- título ----------------
void desenhaTitulo() {
  limpaEcra(C_PRETO);
  // um mundo de fundo
  desenhaMundoPequeno(160, 64, 320, 100, false);
  textoGrande(176 + 4, 8 + 3, "IMPÉRIOS", C_PRETO, 4, 3, &fabgl::FONT_8x16);
  textoGrande(176, 8, "IMPÉRIOS", C_OURO, 4, 3, &fabgl::FONT_8x16);
  textoCentro(0, ECRA_W - 1, 50, "Constrói uma civilização que resista ao tempo", C_BRANCO);
  textoCentro(0, ECRA_W - 1, 190, "FoBoS - ESP32 + FabGL", C_CINZA_E);
}

void entraTitulo() {
  ativa = -1; modoVer = false;
  OpcoesMundo antes = mundo;
  mundo = { 1, 1, 1, 1, 1, 1, 1 };
  poeTamanhoMapa(80, 50);
  geraMundo(millis() | 1);
  mundo = antes;
  memset(mapaVis, 0xFF, MAPA_N);
  desenhaTitulo();
  noTitulo = true;
  TOCA(MUS_TITULO, BAI_TITULO, 120, false);
  limpaLista("", L_TITULO);
  juntaItem("Novo jogo", 0);
  if (haJogoGravado()) juntaItem("Carregar um jogo gravado", 1);
  listaX0 = 190; listaW = 260; listaLinhas = 3;
  listaY0 = 128;                    // a escolha fica por cima do mundo, em baixo
  entraModo(M_LISTA);
  modoAntesLista = M_TITULO;
  desenhaLista();
}

// ---------------- fim do jogo ----------------
void desenhaFim() {
  Serial.printf("[jogo] fim no turno %d: %d (jogador vivo %d)\n", turno, fimDeJogo, CIV[jogador].viva);
  entraModo(M_FIM);
  limpaEcra(fimDeJogo == 1 ? C_PAINEL : RGB888(85, 0, 0));
  char b[200], ano[24];
  textoAno(ano, turno);
  bool vit = fimDeJogo == 1;
  const char * titulo = vit ? "Vitória!" : "Fim da civilização";
  textoGrande((ECRA_W - letras(titulo) * 32) / 2, 16, titulo, C_OURO, 4, 4, &fonte);   // a fonte do jogo tem os acentos
  if (!CIV[jogador].viva) snprintf(b, sizeof(b), "Os %s foram varridos da história em %s%s", nomePovo(jogador), ano,
                                   ano[0] && ano[strlen(ano) - 1] == '.' ? "" : ".");
  else {
    bool outros = false;
    for (int c = 1; c < N_CIVS; ++c) if (c != jogador && CIV[c].viva) outros = true;
    int nave = -1;
    for (int c = 1; c < N_CIVS; ++c) if (CIV[c].viva && CIV[c].naveChega && turno >= CIV[c].naveChega) nave = c;
    if (!outros) snprintf(b, sizeof(b), "Em %s, os %s dominam o mundo inteiro. %s é o senhor de toda a Terra!", ano, nomePovo(jogador), nomeLider(jogador));
    else if (nave == jogador) snprintf(b, sizeof(b), "Em %s, a nave dos %s chegou a Alfa do Centauro. Uma nova era começa entre as estrelas!", ano, nomePovo(jogador));
    else if (nave > 0) snprintf(b, sizeof(b), "Em %s, a nave dos %s chegou a Alfa do Centauro. O futuro é deles.", ano, nomePovo(nave));
    else snprintf(b, sizeof(b), "Chegou o ano %s e a história termina aqui. A tua civilização tem %d pontos.", ano, calculaPontos(jogador));
  }
  textoQuebrado(40, 80, 70, b, C_BRANCO);
  snprintf(b, sizeof(b), "Pontuação: %d   Cidades: %d   Tecnologias: %d", calculaPontos(jogador), contaCidades(jogador), CIV[jogador].nTecs);
  textoCentro(0, ECRA_W - 1, 130, b, C_OURO);
  textoCentro(0, ECRA_W - 1, 180, "ESPAÇO", C_CINZA);
  if (vit) TOCA(MUS_VITORIA, BAI_VITORIA, 110, false); else TOCA(MUS_DERROTA, BAI_DERROTA, 140, false);
}
void atualizaFim() { if (tempo > 60 && carregouFogo()) entraTitulo(); }

void atualizaRelatorio() { if (carregouVolta() || carregouFogo()) { entraModo(M_MAPA); desenhaTudo(); } }
