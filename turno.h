// =====================================================================
// turno.h — IMPÉRIOS: movimento das unidades e o turno de cada povo
// =====================================================================
#pragma once

// ---------------- guerra e paz ----------------
void declaraGuerra(int a, int b) {
  if (a == 0 || b == 0 || a == b) return;
  CIV[a].guerra |= 1 << b; CIV[b].guerra |= 1 << a;
  CIV[a].contacto |= 1 << b; CIV[b].contacto |= 1 << a;
  if (a == jogador) mensagem("Declaraste guerra aos %s.", nomePovo(b));
  else if (b == jogador) mensagem("Os %s declararam-nos guerra!", nomePovo(a));
}
void fazPaz(int a, int b) {
  if (a == 0 || b == 0 || a == b) return;
  CIV[a].guerra &= ~(1 << b); CIV[b].guerra &= ~(1 << a);
}

// ---------------- movimento ----------------
enum ResMov : uint8_t { MV_OK, MV_SEM_MOV, MV_BLOQUEADO, MV_ZOC, MV_GANHOU, MV_PERDEU, MV_PAZ,
                        MV_DIPLOMATA, MV_CARAVANA, MV_CONQUISTOU, MV_EMBARCOU, MV_SEM_LUGAR, MV_DESTRUIU };

int cargaA(int x, int y, int civ, bool aviao) {
  // quanto ainda cabe nos navios (ou porta-aviões) desta casa
  int lista[24], n = unidadesEm(x, y, lista, 24), cap = 0, usados = 0;
  for (int k = 0; k < n; ++k) {
    const Unidade & u = U[lista[k]];
    if (u.civ != civ) continue;
    const InfoUnidade & I = UNIDADE[u.tipo];
    if (I.dominio == D_MAR) {
      if (aviao && (I.flags & U_PORTA_AVIOES)) cap += I.carga;
      if (!aviao && !(I.flags & U_PORTA_AVIOES)) cap += I.carga;
    } else if (I.dominio == (aviao ? D_AR : D_TERRA)) ++usados;
  }
  return cap - usados;
}

// há unidades estrangeiras ao lado de (x, y)? (zona de controlo)
bool estrangeiroAoLado(int civ, int x, int y) {
  for (int d = 0; d < 8; ++d) {
    int nx = x + DIR_DX[d], ny = y + DIR_DY[d];
    if (!dentroY(ny)) continue;
    int c = civNaCasa(nx, ny);
    if (c >= 0 && c != civ && !eOceano(nx, ny)) return true;
  }
  return false;
}

void eliminaPovoSeAcabou(int civ);
void lancaNuclear(int i, int x, int y);

// tenta mover a unidade i na direção d; devolve o resultado
ResMov moveUnidade(int i, int d, bool forcaGuerra = false) {
  Unidade & u = U[i];
  if (u.mov == 0) return MV_SEM_MOV;
  const InfoUnidade & I = UNIDADE[u.tipo];
  int nx = embrulhaX(u.x + DIR_DX[d]), ny = u.y + DIR_DY[d];
  if (!dentroY(ny)) return MV_BLOQUEADO;
  bool mar = eOceano(nx, ny);
  int ci = cidadeEm(nx, ny);
  int outro = civNaCasa(nx, ny);
  bool inimigoNaCasa = outro >= 0 && outro != u.civ;
  bool cidadeAlheia = ci >= 0 && CID[ci].civ != u.civ;

  // --- ataque ---
  if (inimigoNaCasa) {
    if (I.flags & U_DIPLOMATA) { if (cidadeAlheia) return MV_DIPLOMATA; return MV_BLOQUEADO; }
    if (I.flags & U_CARAVANA) { if (ci >= 0) return MV_CARAVANA; return MV_BLOQUEADO; }
    if (I.ataque == 0) return MV_BLOQUEADO;
    if (!emGuerra(u.civ, outro) && !forcaGuerra) return MV_PAZ;
    if (!emGuerra(u.civ, outro)) declaraGuerra(u.civ, outro);
    if (I.dominio == D_TERRA && eOceano(u.x, u.y)) return MV_BLOQUEADO;     // não se ataca de um barco
    if (I.dominio == D_TERRA && mar) return MV_BLOQUEADO;                    // nem para o mar
    if (I.flags & U_NUCLEAR) { lancaNuclear(i, nx, ny); return MV_DESTRUIU; }
    CIV[u.civ].contacto |= 1 << outro; CIV[outro].contacto |= 1 << u.civ;
    bool g = combate(i, nx, ny);
    if (g && outro > 0) eliminaPovoSeAcabou(outro);
    return g ? MV_GANHOU : MV_PERDEU;
  }

  // --- cidade estrangeira sem defesa ---
  if (cidadeAlheia) {
    if (I.flags & U_DIPLOMATA) return MV_DIPLOMATA;
    if (I.flags & U_CARAVANA) return MV_CARAVANA;
    if (I.dominio != D_TERRA || I.ataque == 0) return MV_BLOQUEADO;
    int dono = CID[ci].civ;
    if (!emGuerra(u.civ, dono) && !forcaGuerra) return MV_PAZ;
    if (!emGuerra(u.civ, dono)) declaraGuerra(u.civ, dono);
    if (eOceano(u.x, u.y)) return MV_BLOQUEADO;
    capturaCidade(ci, u.civ);
    u.x = nx; u.y = ny; u.mov = 0; u.estado = EU_NADA;
    if (u.civ > 0) revela(u.civ, nx, ny, 1);
    eliminaPovoSeAcabou(dono);
    return MV_CONQUISTOU;
  }
  if (ci >= 0 && (I.flags & U_CARAVANA) && ci != u.casa) return MV_CARAVANA;

  // --- domínio ---
  int custo = 3;
  bool embarca = false;
  switch (I.dominio) {
    case D_TERRA:
      if (mar) {
        if (cargaA(nx, ny, u.civ, false) <= 0) return MV_SEM_LUGAR;
        embarca = true;
      } else {
        custo = eOceano(u.x, u.y) ? 3 : custoMovimento(u.x, u.y, nx, ny, I.flags & U_TODO_TERRENO);
        // zona de controlo
        if (!(I.flags & (U_DIPLOMATA | U_CARAVANA)) && !eOceano(u.x, u.y) &&
            estrangeiroAoLado(u.civ, u.x, u.y) && estrangeiroAoLado(u.civ, nx, ny) &&
            outro != u.civ && !(ci >= 0 && CID[ci].civ == u.civ)) return MV_ZOC;
      }
      break;
    case D_MAR:
      if (!mar && !(ci >= 0 && CID[ci].civ == u.civ)) return MV_BLOQUEADO;
      custo = 3;
      break;
    case D_AR:
      custo = 3;
      break;
  }
  // com pouco movimento, só às vezes se consegue (como no original)
  if (custo > u.mov && u.mov < movMaximo(i)) {
    if ((int)aleat(custo) >= u.mov) { u.mov = 0; return MV_SEM_MOV; }
    custo = u.mov;
  }
  // os passageiros vão com o navio
  int ox = u.x, oy = u.y;
  if (I.dominio == D_MAR && I.carga > 0) {
    for (int k = 0; k < MAX_UNIDADES; ++k) {
      Unidade & p = U[k];
      if (!p.viva || k == i || p.civ != u.civ || p.x != ox || p.y != oy) continue;
      bool passageiro = (UNIDADE[p.tipo].dominio == D_TERRA && eOceano(ox, oy)) ||
                        (UNIDADE[p.tipo].dominio == D_AR && (I.flags & U_PORTA_AVIOES) && eOceano(ox, oy)) ||
                        (UNIDADE[p.tipo].dominio == D_TERRA && p.estado == EU_SENTINELA && (I.flags & U_PORTA_AVIOES) == 0 &&
                         haCidadeEm(ox, oy) && false);
      if (passageiro) { p.x = nx; p.y = ny; }
    }
  }
  u.x = nx; u.y = ny;
  u.mov = u.mov > custo ? u.mov - custo : 0;
  if (u.estado != EU_DESTINO) u.estado = EU_NADA;
  if (embarca) { u.estado = EU_SENTINELA; u.mov = 0; }
  if (u.civ > 0) {
    revela(u.civ, nx, ny, 1);
    // contacto com quem está ao lado
    for (int k = 0; k < 8; ++k) {
      int c = donoDaCasa(nx + DIR_DX[k], ny + DIR_DY[k]);
      if (c > 0 && c != u.civ && !(CIV[u.civ].contacto >> c & 1)) {
        CIV[u.civ].contacto |= 1 << c; CIV[c].contacto |= 1 << u.civ;
        // primeiro encontro com o jogador: o líder do outro povo aparece e oferece a paz
        if (u.civ == jogador || c == jogador) encontrosPendentes |= 1 << (u.civ == jogador ? c : u.civ);
      }
    }
  }
  if (I.dominio == D_TERRA && temAldeia(nx, ny)) entraAldeia(i);
  return embarca ? MV_EMBARCOU : MV_OK;
}

// ---------------- míssil nuclear ----------------
void lancaNuclear(int i, int x, int y) {
  int civ = U[i].civ;
  // a defesa SDI protege as cidades num raio de 3
  for (int k = 0; k < MAX_CIDADES; ++k)
    if (CID[k].viva && CID[k].civ != civ && temEdif(CID[k], E_SDI) && distancia(CID[k].x, CID[k].y, x, y) <= 3) {
      apagaUnidade(i);
      mensagemEm(civ, x, y, "A defesa SDI de %s abateu o míssil nuclear!", CID[k].nome);
      return;
    }
  apagaUnidade(i);
  for (int dy = -1; dy <= 1; ++dy)
    for (int dx = -1; dx <= 1; ++dx) {
      int xx = x + dx, yy = y + dy;
      if (!dentroY(yy)) continue;
      int lista[24], n = unidadesEm(xx, yy, lista, 24);
      for (int k = 0; k < n; ++k) apagaUnidade(lista[k]);
      if (!eOceano(xx, yy) && aleat(2)) mapaMelh[idxMapa(xx, yy)] |= M_POLUICAO;
    }
  int ci = cidadeEm(x, y);
  if (ci >= 0) CID[ci].tam = max(1, CID[ci].tam / 2);
  mensagemEm(civ, x, y, "Um ataque nuclear devastou %s!", ci >= 0 ? CID[ci].nome : "a região");
  int vit = ci >= 0 ? CID[ci].civ : -1;
  if (vit > 0) { declaraGuerra(civ, vit); for (int c = 1; c < N_CIVS; ++c) if (c != civ && CIV[c].viva && !CIV[c].humano) CIV[c].atitude = 4; }
}

// ---------------- fim de um povo ----------------
void eliminaPovoSeAcabou(int civ) {
  if (civ <= 0 || !CIV[civ].viva) return;
  if (contaCidades(civ) > 0) return;
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].civ == civ && (UNIDADE[U[i].tipo].flags & U_COLONO)) return;
  CIV[civ].viva = 0;
  Serial.printf("[jogo] turno %d: povo %d eliminado\n", turno, civ);
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].civ == civ) apagaUnidade(i);
  if (civ != jogador) mensagem("Os %s foram destruídos! A civilização %s desaparece da história.", nomePovo(civ), adjPovo(civ));
}

// ---------------- cidade: um turno ----------------
uint8_t pedeProducao[MAX_CIDADES];   // cidades do jogador que acabaram uma obra e esperam ordens
int nPedeProducao = 0;

void escolheProducaoIA(int ci);

void acabaProducao(int ci) {
  Cidade & k = CID[ci];
  int p = k.prod, civ = k.civ;
  bool humano = civ == jogador;
  if (p < PROD_EDIF) {
    if (p == UN_COLONOS) {
      if (k.tam <= 1) return;     // espera: uma cidade de 1 não faz colonos
      --k.tam;
    }
    int n = criaUnidade(p, civ, k.x, k.y, ci);
    k.escudos -= UNIDADE[p].custo;
    if (n >= 0 && humano) mensagemEm(civ, k.x, k.y, "%s fez %s.", k.nome, UNIDADE[p].nome);
    arrumaCidade(ci);
    if (!humano) escolheProducaoIA(ci);
    return;
  }
  if (p < PROD_MARAV) {
    int e = p - PROD_EDIF;
    k.escudos -= EDIFICIO[e].custo;
    if (e >= N_EDIFICIOS) {
      ++CIV[civ].nave[e - N_EDIFICIOS];
      if (humano) mensagemEm(civ, k.x, k.y, "%s construiu: %s.", k.nome, EDIFICIO[e].nome);
    } else {
      if (e == E_PALACIO) {   // muda a capital
        int velha = CIV[civ].capital;
        if (velha != SEM && velha < MAX_CIDADES) CID[velha].edif &= ~(1UL << E_PALACIO);
        CIV[civ].capital = ci;
      }
      k.edif |= 1UL << e;
      if (humano) mensagemEm(civ, k.x, k.y, "%s construiu: %s.", k.nome, EDIFICIO[e].nome);
    }
  } else {
    int w = p - PROD_MARAV;
    k.escudos -= MARAVILHA[w].custo;
    maravilhaCidade[w] = ci;
    mensagemEm(civ, k.x, k.y, "%s: os %s acabaram %s!", k.nome, nomePovo(civ), MARAVILHA[w].nome);
    // efeitos imediatos
    if (w == W_DARWIN) {
      for (int v = 0; v < 2; ++v) {
        int cand[N_TECS], n = 0;
        for (int t = 0; t < N_TECS; ++t) if (podePesquisar(civ, t)) cand[n++] = t;
        if (n) ganhaTec(civ, CIV[civ].pesquisa != SEM_TEC && podePesquisar(civ, CIV[civ].pesquisa) ? CIV[civ].pesquisa : cand[0], false);
      }
    }
    if (w == W_MARCO_POLO) for (int c = 1; c < N_CIVS; ++c) if (CIV[c].viva && c != civ) { CIV[civ].contacto |= 1 << c; CIV[c].contacto |= 1 << civ; }
    if (w == W_APOLLO) memset(mapaVis, 0xFF, MAPA_N);
    // quem estava a fazer a mesma maravilha muda de obra
    for (int j = 0; j < MAX_CIDADES; ++j) if (CID[j].viva && j != ci && CID[j].prod == p) {
      if (CID[j].civ == jogador) { mensagemEm(jogador, CID[j].x, CID[j].y, "%s já não pode fazer %s.", CID[j].nome, MARAVILHA[w].nome); pedeProducao[nPedeProducao++ % MAX_CIDADES] = j; }
      else escolheProducaoIA(j);
      CID[j].prod = UN_MILICIA;
    }
  }
  if (humano) { if (nPedeProducao < MAX_CIDADES) pedeProducao[nPedeProducao++] = ci; }
  else escolheProducaoIA(ci);
}

void processaCidade(int ci, int & ouro, int & ciencia) {
  Cidade & k = CID[ci];
  int civ = k.civ;
  k.comprou = 0;
  arrumaCidade(ci);
  RendCidade r = calculaCidade(ci);
  // desordem
  if (r.desordem) {
    ++k.desordem;
    if (civ == jogador) mensagemEm(civ, k.x, k.y, "Desordem em %s! O povo está descontente.", k.nome);
    if (CIV[civ].governo == G_DEMOCRACIA && k.desordem >= 2) {
      mudaGoverno(civ, G_DEMOCRACIA);
      if (civ == jogador) mensagem("A desordem derrubou o governo: o país está em anarquia!");
    }
  } else k.desordem = 0;
  // comida
  k.comida += r.sobraComida;
  int caixa = (k.tam + 1) * 10;
  if (k.comida >= caixa) {
    bool aqueduto = temEdif(k, E_AQUEDUTO);
    if (k.tam >= 10 && !aqueduto) {
      k.comida = caixa;
      if (civ == jogador && aleat(4) == 0) mensagemEm(civ, k.x, k.y, "%s precisa de um aqueduto para crescer.", k.nome);
    } else if (k.tam >= 16 && !temEdif(k, E_METRO) && false) {
      k.comida = caixa;
    } else {
      ++k.tam;
      k.comida = temEdif(k, E_CELEIRO) ? (k.tam + 1) * 5 : 0;
      arrumaCidade(ci);
    }
  } else if (k.comida < 0) {
    // fome: primeiro morre um colono sustentado, depois a cidade encolhe
    bool morreu = false;
    for (int i = 0; i < MAX_UNIDADES && !morreu; ++i)
      if (U[i].viva && U[i].casa == ci && (UNIDADE[U[i].tipo].flags & U_COLONO)) { apagaUnidade(i); morreu = true; }
    if (!morreu) {
      if (k.tam > 1) { --k.tam; arrumaCidade(ci); }
      if (civ == jogador) mensagemEm(civ, k.x, k.y, "Fome em %s!", k.nome);
    } else if (civ == jogador) mensagemEm(civ, k.x, k.y, "Fome em %s: os colonos não têm o que comer.", k.nome);
    k.comida = 0;
  }
  // escudos
  k.escudos += r.sobraEscudos;
  if (k.escudos < 0) {
    // não chega para sustentar as unidades: desfaz-se uma
    for (int i = MAX_UNIDADES - 1; i >= 0; --i)
      if (U[i].viva && U[i].casa == ci) {
        if (civ == jogador) mensagemEm(civ, k.x, k.y, "%s não pode sustentar %s: a unidade foi dispensada.", k.nome, UNIDADE[U[i].tipo].nome);
        apagaUnidade(i); break;
      }
    k.escudos = 0;
  }
  if (!podeProduzir(ci, k.prod) && k.prod >= PROD_EDIF) {
    if (civ == jogador) { if (nPedeProducao < MAX_CIDADES) pedeProducao[nPedeProducao++] = ci; }
    else escolheProducaoIA(ci);
  }
  if (k.escudos >= custoProd(k.prod) && podeProduzir(ci, k.prod)) acabaProducao(ci);
  // poluição
  int sujo = r.escudos - 20 + k.tam / 2;
  if (temEdif(k, E_METRO)) sujo -= k.tam / 2;
  if (temEdif(k, E_RECICLAGEM)) sujo -= 10;
  if (temEdif(k, E_CENTRAL_NUCLEAR) || temEdif(k, E_BARRAGEM) || temMaravilha(civ, W_HOOVER)) sujo -= 5;
  if (sujo > 0 && (int)aleat(100) < sujo * 2) {
    int q = 1 + aleat(20);
    int x = casaX(k, q), y = casaY(k, q);
    if (dentroY(y) && !eOceano(x, y) && !(melh(x, y) & M_POLUICAO)) {
      mapaMelh[idxMapa(x, y)] |= M_POLUICAO;
      if (civ == jogador) mensagemEm(civ, x, y, "Poluição perto de %s! Os colonos podem limpá-la.", k.nome);
    }
  }
  ouro += r.ouro;
  ciencia += r.ciencia;
  // manutenção dos edifícios
  for (int e = 0; e < N_EDIFICIOS; ++e) if (temEdif(k, e)) {
    int m = EDIFICIO[e].manutencao;
    if (m == 1 && temMaravilha(civ, W_ADAM_SMITH)) m = 0;
    ouro -= m;
  }
}

// ---------------- bárbaros ----------------
void apareceBarbaros() {
  if (turno < 110 || aleat(100) > 5) return;      // os bárbaros só aparecem depois de 1800 a.C.
  // perto de uma cidade ao acaso, a 3-5 casas, em terra
  int alvo = -1, n = 0;
  for (int i = 0; i < MAX_CIDADES; ++i) if (CID[i].viva && CID[i].civ > 0 && aleat(++n) == 0) alvo = i;
  if (alvo < 0) return;
  const Cidade & k = CID[alvo];
  for (int t = 0; t < 30; ++t) {
    int x = k.x + aleat(11) - 5, y = k.y + aleat(11) - 5;
    if (!dentroY(y) || eOceano(x, y) || distancia(x, y, k.x, k.y) < 3) continue;
    if (civNaCasa(x, y) >= 0 || haCidadeEm(x, y)) continue;
    int civ = k.civ;
    int tipoA = temTec(civ, TC_CONSCRICAO) ? UN_CAVALARIA : temTec(civ, TC_POLVORA) ? UN_CAVALEIROS : temTec(civ, TC_FERRO) ? UN_LEGIAO : UN_CARRO;
    int nb = 2 + aleat(2) + dificuldade / 2;
    for (int b = 0; b < nb; ++b) criaUnidade(b == 0 && aleat(2) ? UN_CATAPULTA : tipoA, 0, x, y, SEM);
    if (civ == jogador) mensagemEm(0, x, y, "Uma horda de bárbaros apareceu perto de %s!", k.nome);
    return;
  }
}

// ---------------- início do turno de um povo ----------------
void escolhePesquisaIA(int civ);

void inicioTurnoCiv(int civ) {
  Civ & C = CIV[civ];
  if (!C.viva) return;
  // anarquia
  if (C.anarquia > 0) {
    if (--C.anarquia == 0) {
      C.governo = C.govDepois;
      ajustaTaxas(civ);
      if (civ == jogador) mensagem("Acabou a anarquia. O novo governo é: %s.", GOVERNO[C.governo].nome);
    }
  }
  // cidades
  int ouro = 0, ciencia = 0;
  for (int i = 0; i < MAX_CIDADES; ++i) if (CID[i].viva && CID[i].civ == civ) processaCidade(i, ouro, ciencia);
  C.ouro += ouro;
  if (C.ouro < 0) {
    // vende um edifício para pagar as dívidas
    for (int i = 0; i < MAX_CIDADES && C.ouro < 0; ++i) {
      Cidade & k = CID[i];
      if (!k.viva || k.civ != civ) continue;
      for (int e = 1; e < N_EDIFICIOS; ++e) if (temEdif(k, e) && EDIFICIO[e].manutencao > 0) {
        k.edif &= ~(1UL << e);
        C.ouro += EDIFICIO[e].custo;
        if (civ == jogador) mensagemEm(civ, k.x, k.y, "Sem dinheiro: vendemos %s em %s.", EDIFICIO[e].nome, k.nome);
        break;
      }
    }
    if (C.ouro < 0) C.ouro = 0;
  }
  // ciência
  if (civ > 0) {
    // a tecnologia em estudo pode ter chegado por outro lado (aldeia, conquista, Grande Biblioteca)
    if (C.pesquisa != SEM_TEC && !podePesquisar(civ, C.pesquisa)) C.pesquisa = SEM_TEC;
    if (C.pesquisa == SEM_TEC && !C.humano) escolhePesquisaIA(civ);
    C.lampadas += ciencia;
    if (C.pesquisa != SEM_TEC && C.lampadas >= custoTec(civ)) {
      int t = C.pesquisa;
      C.lampadas -= custoTec(civ);
      ganhaTec(civ, t, true);
      if (civ == jogador) mensagem("Os nossos sábios descobriram: %s!", TEC[t].nome);
      C.pesquisa = SEM_TEC;
      if (!C.humano) escolhePesquisaIA(civ);
    }
    // Grande Biblioteca: tecnologias que 2 povos conhecidos já têm
    if (temMaravilha(civ, W_BIBLIOTECA)) {
      for (int t = 0; t < N_TECS; ++t) {
        if (temTec(civ, t)) continue;
        int n = 0;
        for (int c = 1; c < N_CIVS; ++c) if (c != civ && CIV[c].viva && temTec(c, t)) ++n;
        if (n >= 2) { ganhaTec(civ, t, false); if (civ == jogador) mensagem("A Grande Biblioteca dá-nos: %s.", TEC[t].nome); break; }
      }
    }
  }
  // unidades: movimento, trabalhos, combustível
  for (int i = 0; i < MAX_UNIDADES; ++i) {
    Unidade & u = U[i];
    if (!u.viva || u.civ != civ) continue;
    u.mov = movMaximo(i);
    if (u.estado == EU_FORTIFICANDO) u.estado = EU_FORTIFICADA;
    if (u.estado == EU_TRABALHO && u.trabalho != TB_NADA) {
      ++u.prog;
      // colonos na mesma casa e com o mesmo trabalho somam
      if (u.prog >= tempoTrabalho(u.x, u.y, u.trabalho)) acabaTrabalho(i);
      u.mov = 0;
    }
    const InfoUnidade & I = UNIDADE[u.tipo];
    if (I.dominio == D_AR) {
      int cc = cidadeEm(u.x, u.y);
      bool base = (cc >= 0 && CID[cc].civ == civ);
      if (!base) {
        int lista[16], n = unidadesEm(u.x, u.y, lista, 16);
        for (int k = 0; k < n; ++k) if (U[lista[k]].civ == civ && (UNIDADE[U[lista[k]].tipo].flags & U_PORTA_AVIOES)) base = true;
      }
      if (base) u.comb = I.combustivel;
      else if (u.comb <= 1) {
        if (civ == jogador) mensagemEm(civ, u.x, u.y, "%s ficou sem combustível e caiu.", I.nome);
        apagaUnidade(i); continue;
      } else --u.comb;
    }
    // trirremes longe da costa podem perder-se
    if ((I.flags & U_COSTEIRO) && eOceano(u.x, u.y)) {
      bool terraPerto = false;
      for (int d = 0; d < 8; ++d) if (!eOceano(u.x + DIR_DX[d], u.y + DIR_DY[d])) terraPerto = true;
      if (!terraPerto && !temMaravilha(civ, W_FAROL) && aleat(2) == 0) {
        if (civ == jogador) mensagemEm(civ, u.x, u.y, "Uma trirreme perdeu-se no mar alto com quem levava.");
        int lista[16], n = unidadesEm(u.x, u.y, lista, 16);
        for (int k = 0; k < n; ++k) apagaUnidade(lista[k]);
        continue;
      }
    }
  }
  if (civ == 0) apareceBarbaros();
}
