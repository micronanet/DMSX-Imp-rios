// =====================================================================
// ia.h — IMPÉRIOS: os outros povos (e os bárbaros)
// =====================================================================
#pragma once

// ---------------- pesquisa ----------------
// preferências: primeiro o essencial, depois governo, comércio, ciência e guerra
const uint8_t PREF_TEC[] = {
  TC_BRONZE, TC_ALFABETO, TC_CERIMONIAS, TC_ALVENARIA, TC_CODIGO_LEIS, TC_MONARQUIA, TC_ESCRITA, TC_MOEDA,
  TC_FERRO, TC_OLARIA, TC_EQUITACAO, TC_RODA, TC_MATEMATICA, TC_LITERACIA, TC_COMERCIO, TC_FILOSOFIA,
  TC_MISTICISMO, TC_ASTRONOMIA, TC_REPUBLICA, TC_CONSTRUCAO, TC_UNIVERSIDADE, TC_BANCA, TC_FEUDALISMO,
  TC_POLVORA, TC_INVENCAO, TC_NAVEGACAO, TC_FISICA, TC_RELIGIAO, TC_MEDICINA, TC_GRAVIDADE,
};

void escolhePesquisaIA(int civ) {
  Civ & C = CIV[civ];
  for (uint8_t t : PREF_TEC) if (podePesquisar(civ, t) && aleat(3)) { C.pesquisa = t; return; }
  int cand[N_TECS], n = 0;
  for (int t = 0; t < N_TECS; ++t) if (podePesquisar(civ, t)) cand[n++] = t;
  C.pesquisa = n ? cand[aleat(n)] : SEM_TEC;
}

// ---------------- produção ----------------
int melhorDefensor(int ci) {
  int melhor = UN_MILICIA, mv = -1;
  for (int p = 0; p < N_UNIDADES; ++p) {
    if (!podeProduzir(ci, p) || UNIDADE[p].dominio != D_TERRA || !eMilitar(p)) continue;
    int v = UNIDADE[p].defesa * 10 - UNIDADE[p].custo / 10;
    if (v > mv) { mv = v; melhor = p; }
  }
  return melhor;
}
int melhorAtacante(int ci) {
  int melhor = UN_MILICIA, mv = -1;
  for (int p = 0; p < N_UNIDADES; ++p) {
    if (!podeProduzir(ci, p) || UNIDADE[p].dominio != D_TERRA || !eMilitar(p)) continue;
    int v = UNIDADE[p].ataque * 10 + UNIDADE[p].mov * 5 - UNIDADE[p].custo / 10;
    if (v > mv) { mv = v; melhor = p; }
  }
  return melhor;
}

int defensoresEm(int civ, int x, int y) {
  int lista[16], n = unidadesEm(x, y, lista, 16), d = 0;
  for (int k = 0; k < n; ++k) if (U[lista[k]].civ == civ && eMilitar(U[lista[k]].tipo) && UNIDADE[U[lista[k]].tipo].dominio == D_TERRA) ++d;
  return d;
}

int colonosDe(int ci) { int n = 0; for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].casa == ci && (UNIDADE[U[i].tipo].flags & U_COLONO)) ++n; return n; }

bool povoEmGuerra(int civ) {
  for (int c = 1; c < N_CIVS; ++c) if (c != civ && CIV[c].viva && (CIV[civ].guerra >> c & 1)) return true;
  return false;
}

void escolheProducaoIA(int ci) {
  Cidade & k = CID[ci];
  int civ = k.civ;
  RendCidade r = calculaCidade(ci);
  bool guerra = povoEmGuerra(civ) || civ == 0;
  int def = defensoresEm(civ, k.x, k.y);
  int precisa = 1 + (k.tam >= 5) + (guerra ? 1 : 0);
  if (def < precisa) { k.prod = melhorDefensor(ci); return; }
  int nCid = contaCidades(civ);
  int alvoCid = 8 + dificuldade * 2;
  if (civ != 0 && nCid < alvoCid && k.tam >= 2 && colonosDe(ci) < (nCid < 4 ? 2 : 1) && r.sobraComida >= 1 && aleat(4)) { k.prod = UN_COLONOS; return; }
  if (guerra && aleat(3) == 0) { k.prod = melhorAtacante(ci); return; }
  // edifícios, por prioridade
  struct { uint8_t e; bool quer; } lista[] = {
    { E_TEMPLO,       k.tam >= 3 || r.descontentes > 0 },
    { E_CELEIRO,      k.tam >= 4 && aleat(2) },
    { E_MURALHAS,     guerra && k.tam >= 3 },
    { E_BIBLIOTECA,   k.tam >= 3 },
    { E_MERCADO,      k.tam >= 4 },
    { E_PORTO,        k.tam >= 3 && r.sobraComida < 2 },
    { E_TRIBUNAL,     distanciaCapital(k) > 8 && k.tam >= 4 },
    { E_AQUEDUTO,     k.tam >= 8 },
    { E_COLISEU,      k.tam >= 7 || r.descontentes > 1 },
    { E_UNIVERSIDADE, k.tam >= 6 },
    { E_BANCO,        k.tam >= 7 },
    { E_CATEDRAL,     k.tam >= 9 || r.descontentes > 2 },
    { E_FABRICA,      k.tam >= 8 },
    { E_CENTRAL,      k.tam >= 10 },
    { E_METRO,        k.tam >= 12 },
    { E_RECICLAGEM,   k.tam >= 12 },
    { E_QUARTEL,      guerra && aleat(4) == 0 },
  };
  // maravilhas nas cidades boas
  if (civ != 0 && r.escudos >= 6 && aleat(4) == 0) {
    for (int w = 0; w < N_MARAVILHAS; ++w) if (podeProduzir(ci, PROD_MARAV + w)) {
      bool outra = false;
      for (int j = 0; j < MAX_CIDADES; ++j) if (CID[j].viva && j != ci && CID[j].civ == civ && CID[j].prod == PROD_MARAV + w) outra = true;
      if (!outra) { k.prod = PROD_MARAV + w; return; }
    }
  }
  // nave espacial
  if (civ != 0 && podeProduzir(ci, PROD_EDIF + E_NAVE_ESTRUTURA) && r.escudos >= 8) {
    const Civ & C = CIV[civ];
    if (C.nave[0] < 8) { k.prod = PROD_EDIF + E_NAVE_ESTRUTURA; return; }
    if (C.nave[1] < 4 && podeProduzir(ci, PROD_EDIF + E_NAVE_COMPONENTE)) { k.prod = PROD_EDIF + E_NAVE_COMPONENTE; return; }
    if (C.nave[2] < 2 && podeProduzir(ci, PROD_EDIF + E_NAVE_MODULO)) { k.prod = PROD_EDIF + E_NAVE_MODULO; return; }
  }
  for (auto & l : lista) if (l.quer && podeProduzir(ci, PROD_EDIF + l.e)) { k.prod = PROD_EDIF + l.e; return; }
  // barco para explorar, se for costeira e não houver nenhum
  if (juntoAoMar(k.x, k.y) && aleat(6) == 0) {
    for (int p = UN_TRANSPORTE; p >= UN_TRIRREME; --p) if (podeProduzir(ci, p) && UNIDADE[p].carga > 0) { k.prod = p; return; }
  }
  // já tem unidades que cheguem: o edifício mais barato que falte, ou colonos para melhorar o terreno
  int sust = 0;
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].casa == ci) ++sust;
  if (sust >= 2 + k.tam / 3) {
    int melhor = -1, mc = 9999;
    for (int e = 1; e < N_EDIFICIOS; ++e) if (podeProduzir(ci, PROD_EDIF + e) && EDIFICIO[e].custo < mc) { mc = EDIFICIO[e].custo; melhor = e; }
    if (melhor >= 0) { k.prod = PROD_EDIF + melhor; return; }
    if (k.tam >= 3 && colonosDe(ci) == 0) { k.prod = UN_COLONOS; return; }
  }
  k.prod = aleat(2) ? melhorAtacante(ci) : melhorDefensor(ci);
}

// ---------------- movimento ----------------
// dá um passo de i em direção a (tx, ty); devolve o resultado (MV_BLOQUEADO se não conseguiu)
ResMov passoPara(int i, int tx, int ty, bool ataca) {
  Unidade & u = U[i];
  int melhor[8], nm = 0;
  int dAtual = distancia(u.x, u.y, tx, ty);
  // ordena as direções pela distância que fica ao alvo
  int ordem[8], val[8];
  for (int d = 0; d < 8; ++d) {
    int nx = u.x + DIR_DX[d], ny = u.y + DIR_DY[d];
    ordem[d] = d;
    val[d] = dentroY(ny) ? distancia(nx, ny, tx, ty) * 10 + aleat(5) : 9999;
    if (dentroY(ny) && UNIDADE[u.tipo].dominio == D_TERRA) val[d] += TERRENO[terreno(nx, ny)].mov;
  }
  for (int a = 0; a < 8; ++a) for (int b = a + 1; b < 8; ++b) if (val[ordem[b]] < val[ordem[a]]) { int t = ordem[a]; ordem[a] = ordem[b]; ordem[b] = t; }
  (void)melhor; (void)nm;
  for (int k = 0; k < 8; ++k) {
    int d = ordem[k];
    if (val[d] >= 9999) break;
    int nx = u.x + DIR_DX[d], ny = u.y + DIR_DY[d];
    if (distancia(nx, ny, tx, ty) > dAtual + 1) break;        // não se afasta demasiado
    int outro = civNaCasa(nx, ny);
    if (outro >= 0 && outro != u.civ && !ataca) continue;
    if (outro >= 0 && outro != u.civ && !emGuerra(u.civ, outro)) continue;
    int ci = cidadeEm(nx, ny);
    if (ci >= 0 && CID[ci].civ != u.civ && !emGuerra(u.civ, CID[ci].civ)) continue;
    if (UNIDADE[u.tipo].dominio == D_TERRA && eOceano(nx, ny)) continue;
    ResMov r = moveUnidade(i, d);
    if (r != MV_BLOQUEADO && r != MV_ZOC && r != MV_PAZ && r != MV_SEM_LUGAR) return r;
  }
  return MV_BLOQUEADO;
}

// valor de um sítio para fundar uma cidade
int valorSitio(int civ, int x, int y) {
  if (motivoFundar(civ, x, y)) return -1;
  for (int i = 0; i < MAX_CIDADES; ++i) if (CID[i].viva && distancia(CID[i].x, CID[i].y, x, y) < 4) return -1;
  int v = 0;
  for (int r = 0; r < 21; ++r) {
    int xx = x + RAIO_DX[r], yy = y + RAIO_DY[r];
    if (!dentroY(yy)) continue;
    Rend q = rendimentoCasa(civ, xx, yy, r == 0);
    v += q.comida * 3 + q.escudos * 2 + q.comercio;
  }
  uint8_t t = terreno(x, y);
  if (t == T_RIO || t == T_PRADO || t == T_PLANICIE) v += 8;
  if (t == T_COLINAS) v += 6;
  if (juntoAoMar(x, y)) v += 6;
  return v;
}

bool procuraSitio(int i, int & bx, int & by) {
  Unidade & u = U[i];
  int cont = continente(u.x, u.y), melhor = -1;
  for (int dy = -9; dy <= 9; ++dy)
    for (int dx = -9; dx <= 9; ++dx) {
      int x = embrulhaX(u.x + dx), y = u.y + dy;
      if (!dentroY(y) || continente(x, y) != cont) continue;
      int dono = civNaCasa(x, y);
      if (dono >= 0 && dono != u.civ) continue;
      int v = valorSitio(u.civ, x, y);
      if (v < 0) continue;
      v -= distancia(u.x, u.y, x, y) * 3;
      if (v > melhor) { melhor = v; bx = x; by = y; }
    }
  return melhor > 0;
}

// colonos sem sítio: melhoram o terreno à volta da cidade de casa
bool trabalhaTerreno(int i) {
  Unidade & u = U[i];
  int ci = u.casa;
  if (ci == SEM || !CID[ci].viva) return false;
  const Cidade & k = CID[ci];
  int melhorR = -1, melhorTb = TB_NADA, mv = 0;
  for (int r = 1; r < 21; ++r) {
    if (!(k.trab >> r & 1)) continue;
    int x = casaX(k, r), y = casaY(k, r);
    uint8_t t = terreno(x, y), m = melh(x, y);
    if (m & M_POLUICAO) { melhorR = r; melhorTb = TB_LIMPAR; mv = 99; break; }
    int tb = TB_NADA, v = 0;
    if ((t == T_PRADO || t == T_PLANICIE || t == T_DESERTO || t == T_RIO) && !(m & M_REGA) && !motivoTrabalho(u.civ, x, y, TB_REGA)) { tb = TB_REGA; v = 6; }
    else if ((t == T_COLINAS) && !(m & M_MINA)) { tb = TB_MINA; v = 5; }
    else if (!(m & M_ESTRADA) && (t == T_PRADO || t == T_PLANICIE || t == T_DESERTO) && !motivoTrabalho(u.civ, x, y, TB_ESTRADA)) { tb = TB_ESTRADA; v = 3; }
    if (tb == TB_NADA) continue;
    v -= distancia(u.x, u.y, x, y);
    if (civNaCasa(x, y) >= 0 && civNaCasa(x, y) != u.civ) continue;
    if (v > mv) { mv = v; melhorR = r; melhorTb = tb; }
  }
  if (melhorR < 0) return false;
  int x = casaX(k, melhorR), y = casaY(k, melhorR);
  if (u.x == x && u.y == y) {
    if (motivoTrabalho(u.civ, x, y, melhorTb)) return false;
    u.estado = EU_TRABALHO; u.trabalho = melhorTb; u.prog = 0; u.mov = 0;
    return true;
  }
  for (int s = 0; s < 3 && u.viva && u.mov > 0 && !(u.x == x && u.y == y); ++s)
    if (passoPara(i, x, y, false) == MV_BLOQUEADO) break;
  return true;
}

// alvo mais perto para atacar (cidade ou unidade de um povo em guerra), no mesmo continente
bool procuraAlvo(int i, int raio, int & tx, int & ty) {
  Unidade & u = U[i];
  int cont = continente(u.x, u.y), melhor = 9999;
  for (int k = 0; k < MAX_CIDADES; ++k) {
    const Cidade & c = CID[k];
    if (!c.viva || c.civ == u.civ || !emGuerra(u.civ, c.civ)) continue;
    if (UNIDADE[u.tipo].dominio == D_TERRA && continente(c.x, c.y) != cont) continue;
    int d = distancia(u.x, u.y, c.x, c.y);
    if (d <= raio && d < melhor) { melhor = d; tx = c.x; ty = c.y; }
  }
  for (int k = 0; k < MAX_UNIDADES; ++k) {
    const Unidade & e = U[k];
    if (!e.viva || e.civ == u.civ || !emGuerra(u.civ, e.civ)) continue;
    if (UNIDADE[u.tipo].dominio == D_TERRA && (eOceano(e.x, e.y) || continente(e.x, e.y) != cont)) continue;
    int d = distancia(u.x, u.y, e.x, e.y) + 1;
    if (d <= raio && d < melhor) { melhor = d; tx = e.x; ty = e.y; }
  }
  return melhor < 9999;
}

// casa por explorar mais perto (no mesmo continente para unidades de terra)
bool procuraDesconhecido(int i, int & tx, int & ty) {
  Unidade & u = U[i];
  if (u.civ == 0) return false;
  int cont = continente(u.x, u.y);
  bool terra = UNIDADE[u.tipo].dominio == D_TERRA;
  for (int r = 2; r < 16; ++r)
    for (int k = 0; k < 16; ++k) {
      int x = u.x + aleat(r * 2 + 1) - r, y = u.y + aleat(r * 2 + 1) - r;
      if (!dentroY(y) || viu(u.civ, x, y)) continue;
      // tem de haver uma casa conhecida ao lado e acessível
      if (terra && continente(x, y) != cont && !eOceano(x, y)) continue;
      if (!terra && !eOceano(x, y)) continue;
      tx = embrulhaX(x); ty = y; return true;
    }
  return false;
}

int cidadeMaisPerto(int civ, int x, int y) {
  int melhor = -1, md = 9999;
  for (int k = 0; k < MAX_CIDADES; ++k) if (CID[k].viva && CID[k].civ == civ) {
    int d = distancia(x, y, CID[k].x, CID[k].y);
    if (d < md) { md = d; melhor = k; }
  }
  return melhor;
}

void iaUnidade(int i) {
  Unidade & u = U[i];
  const InfoUnidade & I = UNIDADE[u.tipo];
  int civ = u.civ;
  int guarda = 0;
  // ---- colonos ----
  if (I.flags & U_COLONO) {
    if (u.estado == EU_TRABALHO) return;
    // fundar aqui?
    if (valorSitio(civ, u.x, u.y) > 0 && (u.ia == 1 || contaCidades(civ) == 0)) {
      int ci = fundaCidade(civ, u.x, u.y);
      if (ci >= 0) { escolheProducaoIA(ci); apagaUnidade(i); return; }
    }
    int tx, ty;
    if (u.gx != SEM && valorSitio(civ, u.gx, u.gy) > 0) { tx = u.gx; ty = u.gy; }
    else if (contaCidades(civ) < 8 + dificuldade * 2 && procuraSitio(i, tx, ty)) { u.gx = tx; u.gy = ty; }
    else { u.gx = SEM; if (!trabalhaTerreno(i)) u.estado = EU_SENTINELA; return; }
    u.ia = 1;
    for (int s = 0; s < 4 && u.viva && u.mov > 0; ++s) {
      if (u.x == tx && u.y == ty) {
        int ci = fundaCidade(civ, u.x, u.y);
        if (ci >= 0) { escolheProducaoIA(ci); apagaUnidade(i); }
        return;
      }
      if (passoPara(i, tx, ty, false) == MV_BLOQUEADO) { u.gx = SEM; break; }
    }
    return;
  }
  // ---- caravanas: ajudam a fazer maravilhas ----
  if (I.flags & U_CARAVANA) {
    int alvo = -1;
    for (int k = 0; k < MAX_CIDADES; ++k) if (CID[k].viva && CID[k].civ == civ && CID[k].prod >= PROD_MARAV) { alvo = k; break; }
    if (alvo < 0) { u.estado = EU_SENTINELA; return; }
    if (u.x == CID[alvo].x && u.y == CID[alvo].y) { CID[alvo].escudos += 50; apagaUnidade(i); return; }
    for (int s = 0; s < 3 && u.viva && u.mov > 0; ++s) {
      int dx = difX(u.x, CID[alvo].x), dy = CID[alvo].y - u.y;
      if (abs(dx) <= 1 && abs(dy) <= 1) { CID[alvo].escudos += 50; apagaUnidade(i); return; }
      if (passoPara(i, CID[alvo].x, CID[alvo].y, false) == MV_BLOQUEADO) break;
    }
    return;
  }
  if (I.flags & U_DIPLOMATA) { u.estado = EU_FORTIFICADA; return; }
  // ---- aviões: ficam em casa; atacam se houver alvo perto ----
  if (I.dominio == D_AR) {
    int tx, ty;
    if (procuraAlvo(i, I.mov / 2, tx, ty) && distancia(u.x, u.y, tx, ty) <= 1 + I.mov / 3) {
      for (int s = 0; s < I.mov && u.viva && u.mov > 0; ++s) if (passoPara(i, tx, ty, true) != MV_OK) break;
      int cc = cidadeMaisPerto(civ, u.x, u.y);
      if (u.viva && cc >= 0) for (int s = 0; s < I.mov && u.viva && u.mov > 0; ++s) if (passoPara(i, CID[cc].x, CID[cc].y, false) != MV_OK) break;
    }
    return;
  }
  // ---- navios: exploram ----
  if (I.dominio == D_MAR) {
    int tx, ty;
    if (u.estado == EU_SENTINELA && aleat(4)) return;
    if (procuraDesconhecido(i, tx, ty)) {
      for (int s = 0; s < 4 && u.viva && u.mov > 0; ++s) if (passoPara(i, tx, ty, I.ataque > 0) != MV_OK) break;
    } else {
      int cc = cidadeMaisPerto(civ, u.x, u.y);
      if (cc >= 0 && !(u.x == CID[cc].x && u.y == CID[cc].y)) {
        for (int s = 0; s < 4 && u.viva && u.mov > 0; ++s) if (passoPara(i, CID[cc].x, CID[cc].y, false) != MV_OK) break;
      } else u.estado = EU_SENTINELA;
    }
    return;
  }
  // ---- terra: defensores ----
  int ci = cidadeEm(u.x, u.y);
  if (ci >= 0 && CID[ci].civ == civ) {
    int precisa = 1 + (CID[ci].tam >= 5) + (povoEmGuerra(civ) ? 1 : 0);
    // as melhores defesas ficam
    int lista[16], n = unidadesEm(u.x, u.y, lista, 16), melhores = 0;
    for (int k = 0; k < n; ++k) {
      const Unidade & o = U[lista[k]];
      if (o.civ != civ || !eMilitar(o.tipo) || UNIDADE[o.tipo].dominio != D_TERRA) continue;
      if (UNIDADE[o.tipo].defesa > I.defesa || (UNIDADE[o.tipo].defesa == I.defesa && lista[k] < i)) ++melhores;
    }
    if (melhores < precisa) {
      guarda = 1;
      if (u.estado != EU_FORTIFICADA) u.estado = EU_FORTIFICANDO;
      return;
    }
  }
  // ---- terra: atacar, explorar ou ir defender ----
  int tx, ty;
  if (civ == 0 || povoEmGuerra(civ)) {
    if (procuraAlvo(i, civ == 0 ? 20 : 12, tx, ty)) {
      // só ataca se tiver hipótese razoável
      for (int s = 0; s < 3 && u.viva && u.mov > 0; ++s) {
        if (distancia(u.x, u.y, tx, ty) == 1) {
          int d = defensorEm(tx, ty, i);
          if (d >= 0 && forcaAtaque(i) * 3 < forcaDefesa(d, i) && civ != 0) break;
        }
        ResMov r = passoPara(i, tx, ty, true);
        if (r != MV_OK) break;
      }
      return;
    }
  }
  if (I.ataque > 0 && procuraDesconhecido(i, tx, ty) && aleat(3)) {
    for (int s = 0; s < 3 && u.viva && u.mov > 0; ++s) if (passoPara(i, tx, ty, false) != MV_OK) break;
    return;
  }
  // volta para a cidade mais perto que precise
  int melhor = -1, md = 9999;
  for (int k = 0; k < MAX_CIDADES; ++k) {
    if (!CID[k].viva || CID[k].civ != civ) continue;
    if (continente(CID[k].x, CID[k].y) != continente(u.x, u.y)) continue;
    int falta = 1 + (CID[k].tam >= 5) - defensoresEm(civ, CID[k].x, CID[k].y);
    int d = distancia(u.x, u.y, CID[k].x, CID[k].y) - falta * 6;
    if (d < md) { md = d; melhor = k; }
  }
  if (melhor >= 0 && !(u.x == CID[melhor].x && u.y == CID[melhor].y)) {
    for (int s = 0; s < 3 && u.viva && u.mov > 0; ++s) if (passoPara(i, CID[melhor].x, CID[melhor].y, false) != MV_OK) break;
  } else if (!guarda) {
    u.estado = EU_FORTIFICANDO;
  }
}

// ---------------- diplomacia ----------------
// pergunta que um povo faz ao jogador (resolvida no ecrã do mapa)
enum TipoProposta : uint8_t { PR_NADA, PR_PAZ, PR_TRIBUTO, PR_GUERRA };
TipoProposta proposta = PR_NADA;
int propostaCiv = 0, propostaOuro = 0;

int forcaMilitar(int civ) {
  int f = 0;
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].civ == civ) f += UNIDADE[U[i].tipo].ataque + UNIDADE[U[i].tipo].defesa;
  return f;
}

void iaDiplomacia(int civ) {
  Civ & C = CIV[civ];
  bool nacoes = false;
  for (int c = 1; c < N_CIVS; ++c) if (temMaravilha(c, W_NACOES_UNIDAS)) nacoes = true;
  for (int o = 1; o < N_CIVS; ++o) {
    if (o == civ || !CIV[o].viva || !(C.contacto >> o & 1)) continue;
    bool guerra = C.guerra >> o & 1;
    int agress = POVO[C.povo].agressivo + C.atitude / 2;
    int fa = forcaMilitar(civ), fo = forcaMilitar(o);
    if (guerra) {
      int chance = (fa < fo ? 6 : 2) + (nacoes ? 6 : 0);
      if ((int)aleat(100) < chance) {
        if (o == jogador) { if (proposta == PR_NADA) { proposta = PR_PAZ; propostaCiv = civ; } }
        else if (aleat(2)) fazPaz(civ, o);
      }
    } else {
      int chance = agress * (fa > fo ? 2 : 1) - (nacoes ? 2 : 0);
      if (turno < 30) chance = 0;
      if ((int)aleat(400) < chance) {
        if (o == jogador) {
          if (proposta == PR_NADA) {
            if (CIV[o].ouro >= 50 && aleat(2)) { proposta = PR_TRIBUTO; propostaCiv = civ; propostaOuro = min((int)CIV[o].ouro / 2, 50 + (int)aleat(5) * 25); }
            else { proposta = PR_GUERRA; propostaCiv = civ; declaraGuerra(civ, o); }
          }
        } else declaraGuerra(civ, o);
      }
    }
  }
}

// ---------------- o turno de um povo da IA ----------------
void turnoIA(int civ) {
  if (!CIV[civ].viva) return;
  if (civ > 0) {
    // impostos: mais ciência com o tempo, um pouco de luxo nas cidades grandes
    Civ & C = CIV[civ];
    C.luxo = 0;
    C.taxa = C.ouro < 100 ? 5 : 4;
    ajustaTaxas(civ);
    // governo
    if (C.anarquia == 0) {
      int quer = C.governo;
      if (temTec(civ, TC_MONARQUIA) && C.governo == G_DESPOTISMO) quer = G_MONARQUIA;
      if (temTec(civ, TC_REPUBLICA) && !povoEmGuerra(civ) && POVO[C.povo].agressivo == 0 && C.governo < G_REPUBLICA) quer = G_REPUBLICA;
      if (temTec(civ, TC_COMUNISMO) && POVO[C.povo].agressivo == 2 && C.governo == G_MONARQUIA) quer = G_COMUNISMO;
      if (quer != C.governo) mudaGoverno(civ, quer);
    }
    if (C.pesquisa == SEM_TEC) escolhePesquisaIA(civ);
    iaDiplomacia(civ);
    // compra defesas para cidades sem defensores
    for (int k = 0; k < MAX_CIDADES; ++k) {
      Cidade & c = CID[k];
      if (!c.viva || c.civ != civ || c.prod >= PROD_EDIF) continue;
      if (defensoresEm(civ, c.x, c.y) == 0) {
        int falta = custoProd(c.prod) - c.escudos;
        int preco = falta * 2 + falta * falta / 20;
        if (falta > 0 && C.ouro >= preco + 30) { C.ouro -= preco; c.escudos = custoProd(c.prod); }
      }
    }
  }
  for (int i = 0; i < MAX_UNIDADES; ++i) {
    if (!U[i].viva || U[i].civ != civ || U[i].mov == 0) continue;
    if (U[i].estado == EU_FORTIFICADA && aleat(8)) continue;
    if (U[i].estado == EU_TRABALHO) continue;
    iaUnidade(i);
  }
}
