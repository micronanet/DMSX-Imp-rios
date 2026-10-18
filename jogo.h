// =====================================================================
// jogo.h — IMPÉRIOS: o jogo (modos, turno do jogador, menus, cidade,
// relatórios, gravação, fim)
// =====================================================================
#pragma once

enum Modo : uint8_t { M_TITULO, M_LISTA, M_MAPA, M_MENSAGEM, M_PERGUNTA, M_CIDADE, M_RELATORIO, M_DESTINO, M_TAXAS, M_FIM, M_ESPERA, M_ENCONTRO };
Modo modo = M_TITULO;
uint32_t tempo = 0, frame = 0;
void entraModo(Modo m) { modo = m; tempo = 0; }

// o que cada lista está a escolher
enum IdLista : uint8_t { L_TITULO, L_DIFICULDADE, L_NCIVS, L_POVO, L_PRODUCAO, L_PESQUISA, L_ORDENS, L_MENU,
                         L_GOVERNO, L_CIDADES, L_DIPLOMATA, L_CARAVANA, L_OPCOES_CIDADE, L_DIPLOMACIA, L_DIPLO_ACAO, L_VENDER, L_UNID_CIDADE };
Modo modoAntesLista = M_MAPA;
int cidadeAtual = -1;             // cidade aberta (ecrã da cidade) ou a escolher produção
int alvoX = 0, alvoY = 0;          // diplomata/caravana: a cidade onde entrou
int civDiplomacia = 0;

// ---------------- gravação no cartão SD ----------------
const char * FICHEIRO = "/imperios.sav";
const uint32_t MAGICO = 0x31504D49;    // "IMP1"
bool temSD = false;

bool gravaJogo() {
  if (!temSD) return false;
  File f = SD.open(FICHEIRO, FILE_WRITE);
  if (!f) return false;
  uint32_t m = MAGICO;
  f.write((uint8_t *)&m, 4);
  f.write((uint8_t *)&turno, sizeof(turno));
  f.write(&jogador, 1); f.write(&dificuldade, 1); f.write(&nCivsJogo, 1); f.write(&fimDeJogo, 1);
  f.write((uint8_t *)&sementeMapa, sizeof(sementeMapa));
  f.write((uint8_t *)maravilhaCidade, sizeof(maravilhaCidade));
  f.write(tecDescoberta, sizeof(tecDescoberta));
  f.write((uint8_t *)CIV, sizeof(CIV));
  f.write(mapaTer, MAPA_N); f.write(mapaMelh, MAPA_N); f.write(mapaVis, MAPA_N); f.write(mapaCont, MAPA_N);
  f.write((uint8_t *)U, sizeof(Unidade) * MAX_UNIDADES);
  f.write((uint8_t *)CID, sizeof(Cidade) * MAX_CIDADES);
  f.close();
  return true;
}

bool haJogoGravado() {
  if (!temSD) return false;
  File f = SD.open(FICHEIRO, FILE_READ);
  if (!f) return false;
  uint32_t m = 0;
  f.read((uint8_t *)&m, 4);
  f.close();
  return m == MAGICO;
}

bool carregaJogo() {
  if (!temSD) return false;
  File f = SD.open(FICHEIRO, FILE_READ);
  if (!f) return false;
  uint32_t m = 0;
  f.read((uint8_t *)&m, 4);
  if (m != MAGICO) { f.close(); return false; }
  f.read((uint8_t *)&turno, sizeof(turno));
  f.read(&jogador, 1); f.read(&dificuldade, 1); f.read(&nCivsJogo, 1); f.read(&fimDeJogo, 1);
  f.read((uint8_t *)&sementeMapa, sizeof(sementeMapa));
  f.read((uint8_t *)maravilhaCidade, sizeof(maravilhaCidade));
  f.read(tecDescoberta, sizeof(tecDescoberta));
  f.read((uint8_t *)CIV, sizeof(CIV));
  f.read(mapaTer, MAPA_N); f.read(mapaMelh, MAPA_N); f.read(mapaVis, MAPA_N); f.read(mapaCont, MAPA_N);
  f.read((uint8_t *)U, sizeof(Unidade) * MAX_UNIDADES);
  f.read((uint8_t *)CID, sizeof(Cidade) * MAX_CIDADES);
  f.close();
  rngEstado = millis() | 1;
  return true;
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
  geraMundo(millis() * 2654435761UL + 12345);
  memset(U, 0, sizeof(Unidade) * MAX_UNIDADES);
  memset(CID, 0, sizeof(Cidade) * MAX_CIDADES);
  memset(CIV, 0, sizeof(CIV));
  memset(tecDescoberta, 0, sizeof(tecDescoberta));
  for (int w = 0; w < N_MARAVILHAS; ++w) maravilhaCidade[w] = -1;
  turno = 0; fimDeJogo = 0; nMsg = 0; nPedeProducao = 0; proposta = PR_NADA; encontrosPendentes = 0;
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

// segue o destino (ordem "ir para")
void segueDestino(int i) {
  Unidade & u = U[i];
  for (int s = 0; s < 12 && u.viva && u.mov > 0; ++s) {
    if (u.x == u.gx && u.y == u.gy) { u.estado = EU_NADA; u.gx = SEM; return; }
    ResMov r = passoPara(i, u.gx, u.gy, false);
    if (r != MV_OK && r != MV_EMBARCOU) { u.estado = EU_NADA; u.gx = SEM; return; }
    u.estado = EU_DESTINO;
  }
  if (u.viva && u.x == u.gx && u.y == u.gy) { u.estado = EU_NADA; u.gx = SEM; }
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

void abreProducao(int ci, const char * titulo) {
  cidadeAtual = ci;
  modoAntesLista = modo;
  limpaLista(titulo, L_PRODUCAO);
  RendCidade r = calculaCidade(ci);
  char dir[16];
  int sel = 0;
  for (int p = 0; p < N_UNIDADES; ++p) if (podeProduzir(ci, p)) {
    char t[44]; sprintf(t, "%s %d/%d/%d", UNIDADE[p].nome, UNIDADE[p].ataque, UNIDADE[p].defesa, UNIDADE[p].mov);
    sprintf(dir, "%d t.", turnosPara(UNIDADE[p].custo - CID[ci].escudos, r.sobraEscudos));
    if (p == CID[ci].prod) sel = nItens;
    juntaItem(t, p, true, dir);
  }
  for (int e = 0; e < N_EDIFICIOS_TOTAL; ++e) if (podeProduzir(ci, PROD_EDIF + e)) {
    sprintf(dir, "%d t.", turnosPara(EDIFICIO[e].custo - CID[ci].escudos, r.sobraEscudos));
    if (PROD_EDIF + e == CID[ci].prod) sel = nItens;
    juntaItem(EDIFICIO[e].nome, PROD_EDIF + e, true, dir);
  }
  for (int w = 0; w < N_MARAVILHAS; ++w) if (podeProduzir(ci, PROD_MARAV + w)) {
    char t[44]; snprintf(t, sizeof(t), "*%s", MARAVILHA[w].nome);
    sprintf(dir, "%d t.", turnosPara(MARAVILHA[w].custo - CID[ci].escudos, r.sobraEscudos));
    if (PROD_MARAV + w == CID[ci].prod) sel = nItens;
    juntaItem(t, PROD_MARAV + w, true, dir);
  }
  itemSel = sel;
  listaX0 = 100; listaW = 380; listaLinhas = 16;
  strcpy(ajudaLista, "* = maravilha do mundo   ESC: manter");
  entraModo(M_LISTA);
  desenhaLista();
}

void abrePesquisa() {
  modoAntesLista = modo;
  limpaLista("Os sábios perguntam: o que devemos estudar?", L_PESQUISA);
  for (int t = 0; t < N_TECS; ++t) if (podePesquisar(jogador, t)) juntaItem(TEC[t].nome, t);
  if (nItens == 0) { CIV[jogador].pesquisa = SEM_TEC; return; }
  char b[40]; sprintf(b, "Custo: %d lâmpadas", custoTec(jogador));
  strcpy(ajudaLista, b);
  listaX0 = 120; listaW = 300; listaLinhas = 15;
  entraModo(M_LISTA);
  desenhaLista();
}

// ordens da unidade ativa (para quem joga com o joystick, ou ENTER)
enum Ordem { O_FUNDAR, O_JUNTAR, O_ESTRADA, O_FERROVIA, O_REGA, O_MINA, O_FORTALEZA, O_LIMPAR, O_FORTIFICAR,
             O_SENTINELA, O_IR, O_ESPERAR, O_NADA, O_CASA, O_DISPENSAR, O_VER_CIDADE, O_ACORDAR, O_FIM_TURNO, O_VER, O_MENU };
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
    juntaItem("Esperar", O_ESPERAR, true, "W");
    juntaItem("Sem ordens", O_NADA, true, "ESPAÇO");
    if (ci >= 0 && CID[ci].civ == jogador) {
      juntaItem("Mudar a casa para aqui", O_CASA, u.casa != ci, "H");
      juntaItem("Ver a cidade", O_VER_CIDADE, true, "C");
    }
    int lista[16], n = unidadesEm(u.x, u.y, lista, 16);
    if (n > 1) juntaItem("Acordar as outras", O_ACORDAR, true, "U");
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
            MN_NAVE, MN_GRAVAR, MN_MUSICA, MN_SAIR, MN_VOLTAR };
bool podeLancarNave(int civ) {
  const Civ & C = CIV[civ];
  return C.naveChega == 0 && C.nave[0] >= 4 && C.nave[1] >= 2 && C.nave[2] >= 1 && C.capital != SEM;
}
void abreMenu() {
  modoAntesLista = M_MAPA;
  limpaLista("Menu do jogo", L_MENU);
  juntaItem("As nossas cidades", MN_CIDADES, true, "F1");
  juntaItem("Impostos, luxo e ciência", MN_TAXAS, true, "T");
  juntaItem("Revolução (mudar de governo)", MN_REVOLUCAO, CIV[jogador].anarquia == 0, "X");
  juntaItem("Diplomacia", MN_DIPLOMACIA, true, "Y");
  juntaItem("Conselho de ciência", MN_CIENCIA, true, "F6");
  juntaItem("Os povos do mundo", MN_POVOS, true, "F3");
  juntaItem("Maravilhas do mundo", MN_MARAVILHAS, true, "F7");
  juntaItem("Pontuação", MN_PONTOS, true, "F9");
  if (podeLancarNave(jogador)) juntaItem("Lançar a nave espacial!", MN_NAVE, true);
  juntaItem("Gravar o jogo", MN_GRAVAR, temSD, "F10");
  juntaItem(musicaPermitida ? "Música: ligada" : "Música: desligada", MN_MUSICA, true);
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
        texto(136, y, POVO[CIV[c].povo].lider, C_BRANCO);
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

// rendimento de uma casa no mapa da cidade: comida, escudos e comércio com um quadradinho de cor
void desenhaRendimento(int sx, int sy, Rend v) {
  retangulo(sx + 2, sy + TILE_H - 10, sx + TILE_W - 3, sy + TILE_H - 2, C_PRETO);
  const RGB888 cor[3] = { C_VERDE, C_OURO, C_AMARELO };
  int val[3] = { v.comida, v.escudos, v.comercio };
  for (int i = 0; i < 3; ++i) {
    int x = sx + 4 + i * 14, y = sy + TILE_H - 9;
    retangulo(x, y + 1, x + 3, y + 6, cor[i]);
    char t[12]; sprintf(t, "%d", min(9, (int)val[i]));
    texto(x + 5, y, t, cor[i]);
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
  sprintf(b, "Comércio %3d (corr. %d)", r.comercio, r.corrupcao);
  texto(8, 149, b, C_AMARELO);
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
    poeBarra("Modo ver: setas movem o cursor; ESPAÇO abre a cidade.", "ESC ou V: voltar às unidades.");
  }
}

void continuaTurno();
void mostraDepoisDeAcao() {
  if (!mostraMensagens()) { entraModo(M_MAPA); desenhaTudo(); if (encontrosPendentes) continuaTurno(); }
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
      if (ci >= 0) { somTri(784, 200, 50); abreProducao(ci, "Nova cidade! O que vamos fazer?"); return; }
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
      poeBarra("Ir para: escolhe o destino com as setas e ESPAÇO.", "ESC: cancelar.");
      return;
    case O_ESPERAR: { int a = ativa; proximaUnidade(); if (ativa == a) ativa = a; break; }
    case O_NADA: u.mov = 0; break;
    case O_CASA: { int ci = cidadeEm(u.x, u.y); if (ci >= 0) u.casa = ci; break; }
    case O_DISPENSAR: apagaUnidade(ativa); ativa = -1; break;
    case O_VER_CIDADE: { int ci = cidadeEm(u.x, u.y); if (ci >= 0) { abreCidade(ci); return; } break; }
    case O_ACORDAR: {
      int lista[16], n = unidadesEm(u.x, u.y, lista, 16);
      for (int k = 0; k < n; ++k) if (U[lista[k]].civ == jogador && U[lista[k]].estado != EU_TRABALHO) U[lista[k]].estado = EU_NADA;
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
  if (encontrosPendentes) continuaTurno();          // primeiro encontro com outro povo
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
  if (premiu(VK_F3)) { abreRelatorio(MN_POVOS); return; }
  if (premiu(VK_F6)) { abreRelatorio(MN_CIENCIA); return; }
  if (premiu(VK_F7)) { abreRelatorio(MN_MARAVILHAS); return; }
  if (premiu(VK_F9)) { abreRelatorio(MN_PONTOS); return; }
  if (premiu(VK_F10)) { if (gravaJogo()) poeBarra("Jogo gravado no cartão SD.", ""); else poeBarra("Não foi possível gravar (cartão SD?).", ""); return; }
  if (premiuLetra('T')) { taxaSel = 0; entraModo(M_TAXAS); desenhaTaxas(); return; }
  if (premiuLetra('X')) { if (CIV[jogador].anarquia == 0) abreGoverno(); return; }
  if (premiuLetra('Y')) { abreDiplomacia(); return; }
  if (premiuLetra('V')) { mudaParaVer(true); desenhaTudo(); poeBarra("Modo ver: setas movem o cursor; ESPAÇO abre a cidade.", "ESC ou V: voltar às unidades."); return; }
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
  if (premiuLetra('G')) { executaOrdem(O_IR); return; }
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

void atualizaDestino() {
  int d = direcaoPedida();
  if (d >= 0) {
    cursorX = embrulhaX(cursorX + DIR_DX[d]); cursorY = constrain(cursorY + DIR_DY[d], 0, MAPA_H - 1);
    if (!naVista(cursorX, cursorY, 1)) centraEm(cursorX, cursorY);
    desenhaMapa();
  }
  if (premiu(VK_ESCAPE) || joyVolta()) { modoVer = false; entraModo(M_MAPA); desenhaTudo(); poeBarra("", ""); return; }
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
void acabaTurnoJogador() {
  poeBarra("Os outros povos estão a jogar...", "");
  canvas.waitCompletion(false);
  ativa = -1;
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
  ++turno;
  if (turno % 10 == 0) { Serial.printf("[jogo] turno %d:", turno); for (int c = 1; c < N_CIVS; ++c) if (CIV[c].viva) Serial.printf(" %d:%dc/%du", c, contaCidades(c), contaUnidades(c)); Serial.printf(" bárb %du\n", contaUnidades(0)); }
  if (verificaFim()) { desenhaFim(); return; }
  if (turno % 10 == 0 && temSD) gravaJogo();
  comecaTurnoJogador();
}

void comecaTurnoJogador() {
  inicioTurnoCiv(jogador);
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
  desenhaArte(240, 22, arte_retrato[CIV[c].povo % N_RETRATOS], RETRATO_W, RETRATO_H);
  // o que o líder diz
  caixa(4, 113, ECRA_W - 5, 197, C_PAINEL, C_MOLDURA);
  snprintf(b, sizeof(b), "%s, líder dos %s", POVO[CIV[c].povo].lider, nomePovo(c));
  textoCentro(4, ECRA_W - 5, 118, b, cc);
  const char * tom[3] = {
    "O nosso povo ama a paz e o comércio.",
    "Respeitamos quem nos respeita.",
    "Os nossos exércitos são temidos em toda a parte, mas hoje estendemos-vos a mão." };
  snprintf(b, sizeof(b), "\"Saudações, %s, chefe dos %s! %s Proponho que os nossos povos vivam em paz. Aceitas?\"",
           POVO[CIV[jogador].povo].lider, nomePovo(jogador), tom[POVO[CIV[c].povo].agressivo % 3]);
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

// o que falta mostrar no início do turno: mensagens, propostas, pesquisa, produção
void continuaTurno() {
  if (nMsg > 0) { mostraMensagens(); return; }
  while (encontrosPendentes) {
    int c = 0;
    while (!(encontrosPendentes >> c & 1)) ++c;
    encontrosPendentes &= ~(1 << c);
    if (c > 0 && c != jogador && CIV[c].viva) { abreEncontro(c); return; }
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

void escolhaLista(int s) {
  int v = s >= 0 ? itens[s].valor : -1;
  switch (idLista) {
    case L_TITULO:
      if (s < 0) { entraTitulo(); return; }
      if (v == 1 && carregaJogo()) {
        ativa = -1; ultimaAtiva = -1;
        proximaUnidade();
        if (ativa >= 0) centraEm(U[ativa].x, U[ativa].y);
        else if (CIV[jogador].capital != SEM) centraEm(CID[CIV[jogador].capital].x, CID[CIV[jogador].capital].y);
        TOCA(MUS_TITULO, BAI_TITULO, 120, false);
        entraModo(M_MAPA); desenhaTudo(); continuaTurno(); return;
      }
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
      limpaLista("Escolhe o teu povo", L_POVO);
      for (int p = 0; p < N_POVOS; ++p) juntaItem(POVO[p].nome, p, true, POVO[p].lider);
      listaX0 = 140; listaW = 360; listaLinhas = 15;
      desenhaLista();
      return;
    case L_POVO: {
      if (s < 0) { entraTitulo(); return; }
      povoEscolhido = v;
      limpaEcra(C_PRETO);
      textoCentro(0, ECRA_W - 1, 90, "A criar o mundo...", C_AMARELO);
      canvas.waitCompletion(false);
      novoJogo();
      char b[300];
      snprintf(b, sizeof(b), "No ano 4000 a.C., o povo dos %s, guiado por %s, deixa a vida de nómadas e procura um lugar para a primeira cidade. "
               "Usa os colonos para fundar cidades (B), faz crescer o império, descobre tecnologias e constrói maravilhas. "
               "Conquista o mundo, ou chega primeiro às estrelas.",
               POVO[povoEscolhido].nome, POVO[povoEscolhido].lider);
      mensagem("%s", b);
      TOCA(MUS_TITULO, BAI_TITULO, 120, false);
      comecaTurnoJogador();
      return;
    }
    case L_PRODUCAO:
      if (s >= 0 && cidadeAtual >= 0) {
        Cidade & k = CID[cidadeAtual];
        // mudar entre unidades, edifícios e maravilhas custa metade do que já estava feito (como no original)
        int antes = k.prod < PROD_EDIF ? 0 : k.prod < PROD_MARAV ? 1 : 2;
        int depois = v < PROD_EDIF ? 0 : v < PROD_MARAV ? 1 : 2;
        if (antes != depois && k.escudos > 0) k.escudos /= 2;
        k.prod = v;
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
        case MN_CIENCIA: case MN_POVOS: case MN_MARAVILHAS: case MN_PONTOS: abreRelatorio(v); return;
        case MN_NAVE: desenhaTudo(); pergunta(Q_NAVE, "Lançar a nave com %d estruturas, %d componentes e %d módulos?", CIV[jogador].nave[0], CIV[jogador].nave[1], CIV[jogador].nave[2]); return;
        case MN_GRAVAR: desenhaTudo(); poeBarra(gravaJogo() ? "Jogo gravado no cartão SD." : "Não foi possível gravar.", ""); return;
        case MN_MUSICA: musicaPermitida = !musicaPermitida; desenhaTudo(); return;
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
  int s = atualizaLista();
  if (s == -1) return;
  escolhaLista(s == -2 ? -1 : s);
}

// ---------------- título ----------------
void desenhaTitulo() {
  limpaEcra(C_PRETO);
  // um mundo de fundo (2x1 por casa, ampliado 4x2)
  for (int y = 0; y < MAPA_H; ++y) {
    int x = 0;
    while (x < MAPA_W) {
      uint8_t t = terreno(x, y);
      int x2 = x + 1;
      while (x2 < MAPA_W && terreno(x2, y) == t) ++x2;
      canvas.setBrushColor(corMinimapa(t));
      canvas.fillRectangle(160 + x * 4, 64 + y * 2, 160 + x2 * 4 - 1, 64 + y * 2 + 1);
      x = x2;
    }
  }
  textoGrande(176 + 4, 8 + 3, "IMPÉRIOS", C_PRETO, 4, 3, &fabgl::FONT_8x16);
  textoGrande(176, 8, "IMPÉRIOS", C_OURO, 4, 3, &fabgl::FONT_8x16);
  textoCentro(0, ECRA_W - 1, 50, "Constrói uma civilização que resista ao tempo", C_BRANCO);
  textoCentro(0, ECRA_W - 1, 190, "FoBoS - ESP32 + FabGL", C_CINZA_E);
}

void entraTitulo() {
  ativa = -1; modoVer = false;
  geraMundo(millis() | 1);
  memset(mapaVis, 0xFF, MAPA_N);
  desenhaTitulo();
  TOCA(MUS_TITULO, BAI_TITULO, 120, true);
  limpaLista("", L_TITULO);
  juntaItem("Novo jogo", 0);
  if (haJogoGravado()) juntaItem("Continuar o jogo gravado", 1);
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
  textoGrande((ECRA_W - letras(titulo) * 32) / 2, 16, titulo, C_OURO, 4, 3, &fabgl::FONT_8x16);
  if (!CIV[jogador].viva) snprintf(b, sizeof(b), "Os %s foram varridos da história em %s.", nomePovo(jogador), ano);
  else {
    bool outros = false;
    for (int c = 1; c < N_CIVS; ++c) if (c != jogador && CIV[c].viva) outros = true;
    int nave = -1;
    for (int c = 1; c < N_CIVS; ++c) if (CIV[c].viva && CIV[c].naveChega && turno >= CIV[c].naveChega) nave = c;
    if (!outros) snprintf(b, sizeof(b), "Em %s, os %s dominam o mundo inteiro. %s é o senhor de toda a Terra!", ano, nomePovo(jogador), POVO[CIV[jogador].povo].lider);
    else if (nave == jogador) snprintf(b, sizeof(b), "Em %s, a nave dos %s chegou a Alfa do Centauro. Uma nova era começa entre as estrelas!", ano, nomePovo(jogador));
    else if (nave > 0) snprintf(b, sizeof(b), "Em %s, a nave dos %s chegou a Alfa do Centauro. O futuro é deles.", ano, nomePovo(nave));
    else snprintf(b, sizeof(b), "Chegou o ano %s e a história termina aqui. A tua civilização tem %d pontos.", ano, calculaPontos(jogador));
  }
  textoQuebrado(40, 80, 70, b, C_BRANCO);
  snprintf(b, sizeof(b), "Pontuação: %d   Cidades: %d   Tecnologias: %d", calculaPontos(jogador), contaCidades(jogador), CIV[jogador].nTecs);
  textoCentro(0, ECRA_W - 1, 130, b, C_OURO);
  textoCentro(0, ECRA_W - 1, 180, "ESPAÇO", C_CINZA);
  if (vit) TOCA(MUS_VITORIA, BAI_VITORIA, 110, false); else TOCA(MUS_DERROTA, BAI_DERROTA, 140, false);
  if (temSD) SD.remove(FICHEIRO);
}
void atualizaFim() { if (tempo > 60 && carregouFogo()) entraTitulo(); }

void atualizaRelatorio() { if (carregouVolta() || carregouFogo()) { entraModo(M_MAPA); desenhaTudo(); } }
