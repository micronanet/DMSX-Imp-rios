// =====================================================================
// regras.h — IMPÉRIOS: as regras do jogo
//  mensagens · unidades (movimento, combate, colonos, aldeias)
//  cidades (casas, comida, escudos, comércio, felicidade, produção)
//  ciência, governos, pontuação
// =====================================================================
#pragma once

// ---------------- mensagens para o jogador ----------------
const int MAX_MSG = 12;
struct Mensagem { char texto[320]; int8_t civ; int8_t x, y; };   // x, y: onde aconteceu (-1 = em lado nenhum)
Mensagem filaMsg[MAX_MSG];
int nMsg = 0;

void mensagemEm(int civ, int x, int y, const char * fmt, ...) {
  if (nMsg >= MAX_MSG) return;
  va_list ap; va_start(ap, fmt);
  vsnprintf(filaMsg[nMsg].texto, sizeof(filaMsg[nMsg].texto), fmt, ap);
  va_end(ap);
  filaMsg[nMsg].civ = civ; filaMsg[nMsg].x = x; filaMsg[nMsg].y = y;
  ++nMsg;
}
#define mensagem(...) mensagemEm(-1, -1, -1, __VA_ARGS__)

const char * nomePovo(int c) { return c == 0 ? "Bárbaros" : POVO[CIV[c].povo].nome; }
const char * adjPovo(int c)  { return c == 0 ? "bárbaro" : POVO[CIV[c].povo].adjetivo; }
int corDe(int c) { return c == 0 ? 0 : POVO[CIV[c].povo].cor; }

// ---------------- procuras ----------------
int cidadeEm(int x, int y) {
  x = embrulhaX(x);
  for (int i = 0; i < MAX_CIDADES; ++i) if (CID[i].viva && CID[i].x == x && CID[i].y == y) return i;
  return -1;
}
bool haCidadeEm(int x, int y) { return cidadeEm(x, y) >= 0; }

int unidadesEm(int x, int y, int * lista, int maxN) {
  x = embrulhaX(x);
  int n = 0;
  for (int i = 0; i < MAX_UNIDADES && n < maxN; ++i) if (U[i].viva && U[i].x == x && U[i].y == y) lista[n++] = i;
  return n;
}
int primeiraUnidadeEm(int x, int y) {
  x = embrulhaX(x);
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].x == x && U[i].y == y) return i;
  return -1;
}
// povo das unidades numa casa (-1 = nenhuma)
int civNaCasa(int x, int y) { int i = primeiraUnidadeEm(x, y); return i < 0 ? -1 : U[i].civ; }
int donoDaCasa(int x, int y) { int c = cidadeEm(x, y); if (c >= 0) return CID[c].civ; return civNaCasa(x, y); }

int contaCidades(int civ) { int n = 0; for (int i = 0; i < MAX_CIDADES; ++i) if (CID[i].viva && CID[i].civ == civ) ++n; return n; }
int contaUnidades(int civ) { int n = 0; for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].civ == civ) ++n; return n; }

bool temMaravilha(int civ, int w) {
  int c = maravilhaCidade[w];
  if (c < 0 || !CID[c].viva || CID[c].civ != civ) return false;
  if (MARAVILHA[w].obsoleta != SEM_TEC) {
    // fica obsoleta quando alguém descobre a tecnologia
    for (int k = 1; k < N_CIVS; ++k) if (CIV[k].viva && temTec(k, MARAVILHA[w].obsoleta)) return false;
  }
  return true;
}
bool cidadeTemMaravilha(int ci, int w) { return maravilhaCidade[w] == ci && temMaravilha(CID[ci].civ, w); }

// ---------------- ciência ----------------
int custoTec(int civ) {
  int f = CIV[civ].humano ? 6 + dificuldade * 2 : 14 - dificuldade * 2;
  return (CIV[civ].nTecs + 1) * f;
}
bool podePesquisar(int civ, int t) {
  if (temTec(civ, t)) return false;
  return temTec(civ, TEC[t].pre1) && temTec(civ, TEC[t].pre2);
}
int contaPesquisaveis(int civ) { int n = 0; for (int t = 0; t < N_TECS; ++t) if (podePesquisar(civ, t)) ++n; return n; }

void escolhePesquisaIA(int civ);
// tecnologias novas do jogador, à espera de mostrar a janela da descoberta
const int MAX_TECS_NOVAS = 8;
uint8_t tecsNovas[MAX_TECS_NOVAS], tecsNovasSabios[MAX_TECS_NOVAS];
int nTecsNovas = 0;
void ganhaTec(int civ, int t, bool anuncia) {
  if (t == SEM_TEC) return;
  if (CIV[civ].pesquisa == t) CIV[civ].pesquisa = SEM_TEC;
  if (temTec(civ, t)) return;
  poeTec(civ, t);
  ++CIV[civ].nTecs;
  if (tecDescoberta[t] == 0) tecDescoberta[t] = civ;
  if (civ == jogador && nTecsNovas < MAX_TECS_NOVAS) { tecsNovasSabios[nTecsNovas] = 0; tecsNovas[nTecsNovas++] = t; }
  if (CIV[civ].pesquisa == t) { CIV[civ].pesquisa = SEM_TEC; CIV[civ].lampadas = 0; }
  if (anuncia && civ != jogador && (CIV[jogador].contacto >> civ & 1))
    mensagem("Os %s descobriram: %s.", nomePovo(civ), TEC[t].nome);
}

// ---------------- unidades ----------------
int criaUnidade(int tipo, int civ, int x, int y, int casa) {
  for (int i = 0; i < MAX_UNIDADES; ++i) {
    if (U[i].viva) continue;
    Unidade & u = U[i];
    memset(&u, 0, sizeof(u));
    u.viva = 1; u.tipo = tipo; u.civ = civ; u.x = embrulhaX(x); u.y = y;
    u.mov = UNIDADE[tipo].mov * 3; u.casa = casa; u.gx = SEM;
    u.comb = UNIDADE[tipo].combustivel;
    if (casa != SEM && casa < MAX_CIDADES && CID[casa].viva && temEdif(CID[casa], E_QUARTEL)) u.vet = 1;
    if (UNIDADE[tipo].dominio == D_MAR && temMaravilha(civ, W_FAROL)) u.vet = 1;
    if (civ > 0) revela(civ, x, y, 1);
    return i;
  }
  return -1;
}

void apagaUnidade(int i) { U[i].viva = 0; }

int movMaximo(int i) {
  const Unidade & u = U[i];
  int m = UNIDADE[u.tipo].mov;
  if (UNIDADE[u.tipo].dominio == D_MAR && temMaravilha(u.civ, W_MAGALHAES)) m += 2;
  return m * 3;
}

bool eMilitar(int tipo) { return UNIDADE[tipo].ataque > 0 && !(UNIDADE[tipo].flags & (U_COLONO | U_DIPLOMATA | U_CARAVANA)); }

// força de defesa de uma unidade na sua casa (vezes 8 para não usar números com vírgula)
int forcaDefesa(int d, int atacante) {
  const Unidade & u = U[d];
  int f = UNIDADE[u.tipo].defesa * 8;
  uint8_t t = terreno(u.x, u.y);
  if (UNIDADE[u.tipo].dominio == D_TERRA) f = f * TERRENO[t].defesa / 2;
  if (u.vet) f = f * 3 / 2;
  int ci = cidadeEm(u.x, u.y);
  bool forte = melh(u.x, u.y) & M_FORTALEZA;
  if (forte) f *= 2;
  else if (u.estado == EU_FORTIFICADA || ci >= 0) f = f * 3 / 2;
  if (ci >= 0 && atacante >= 0 && UNIDADE[U[atacante].tipo].dominio == D_TERRA) {
    bool muralha = temEdif(CID[ci], E_MURALHAS) || temMaravilha(CID[ci].civ, W_MURALHA);
    if (muralha) f *= 3;
  }
  if (ci >= 0 && atacante >= 0 && UNIDADE[U[atacante].tipo].dominio == D_AR && temEdif(CID[ci], E_SDI)) f *= 2;
  // navios no mar contra unidades em terra e vice-versa não somam terreno; aviões só são atacados por caças
  return max(1, f);
}
int forcaAtaque(int a) {
  int f = UNIDADE[U[a].tipo].ataque * 8;
  if (U[a].vet) f = f * 3 / 2;
  return f;
}

// o melhor defensor de uma casa contra o atacante
int defensorEm(int x, int y, int atacante) {
  int lista[16], n = unidadesEm(x, y, lista, 16), melhor = -1, mf = -1;
  for (int k = 0; k < n; ++k) {
    int d = lista[k];
    // um avião só pode ser atacado por caças
    if (UNIDADE[U[d].tipo].dominio == D_AR && atacante >= 0 && U[atacante].tipo != UN_CACA && n > 1) continue;
    int f = forcaDefesa(d, atacante);
    if (f > mf) { mf = f; melhor = d; }
  }
  return melhor;
}

void capturaCidade(int ci, int novo);

// combate: devolve true se o atacante ganhou. O vencido desaparece; fora das cidades e fortalezas
// caem todas as unidades da casa (como no original).
bool combate(int a, int x, int y) {
  int d = defensorEm(x, y, a);
  if (d < 0) return true;
  int civA = U[a].civ, civD = U[d].civ;
  int tipoA = U[a].tipo, tipoD = U[d].tipo;   // (as unidades podem desaparecer no combate)
  int fa = forcaAtaque(a), fd = forcaDefesa(d, a);
  bool ganhou = (int)(aleat() % (uint32_t)(fa + fd)) < fa;
  // missão: uma unidade gasta 1 ponto a atacar
  U[a].mov = U[a].mov >= 3 ? U[a].mov - 3 : 0;
  int ci = cidadeEm(x, y);
  if (ganhou) {
    bool protegidos = ci >= 0 || (melh(x, y) & M_FORTALEZA);
    if (protegidos) apagaUnidade(d);
    else {
      int lista[16], n = unidadesEm(x, y, lista, 16);
      for (int k = 0; k < n; ++k) apagaUnidade(lista[k]);
    }
    if (!U[a].vet && aleat(2) == 0) U[a].vet = 1;
    if (ci >= 0 && CID[ci].tam > 1 && UNIDADE[U[a].tipo].dominio != D_AR && aleat(2) == 0) --CID[ci].tam;
  } else {
    apagaUnidade(a);
    if (!U[d].vet && aleat(2) == 0) U[d].vet = 1;
  }
  // avisos para o jogador
  if (civD == jogador) {
    const char * onde = ci >= 0 ? CID[ci].nome : TERRENO[terreno(x, y)].nome;
    if (ganhou) mensagemEm(civA, x, y, "%s (%s) atacou-nos em %s: perdemos %s.", UNIDADE[tipoA].nome, adjPovo(civA), onde, UNIDADE[tipoD].nome);
    else mensagemEm(civA, x, y, "%s (%s) atacou-nos em %s e foi derrotado pelos nossos %s.", UNIDADE[tipoA].nome, adjPovo(civA), onde, UNIDADE[tipoD].nome);
  }
  return ganhou;
}

// ---------------- colonos: trabalhos no terreno ----------------
int tempoTrabalho(int x, int y, int tb) {
  const InfoTerreno & I = TERRENO[terreno(x, y)];
  switch (tb) {
    case TB_ESTRADA:   return I.tEstrada;
    case TB_FERROVIA:  return I.tEstrada * 2;
    case TB_REGA:      return I.tRega;
    case TB_MINA:      return I.tMina;
    case TB_FORTALEZA: return 3;
    case TB_LIMPAR:    return 3;
  }
  return 0;
}

// pode-se fazer o trabalho tb na casa? (devolve um motivo, ou nullptr se pode)
const char * motivoTrabalho(int civ, int x, int y, int tb) {
  uint8_t t = terreno(x, y), m = melh(x, y);
  const InfoTerreno & I = TERRENO[t];
  switch (tb) {
    case TB_ESTRADA:
      if (m & M_ESTRADA) return "Já há estrada aqui.";
      if (t == T_RIO && !temTec(civ, TC_PONTES)) return "Para fazer estradas em rios é preciso Pontes.";
      return nullptr;
    case TB_FERROVIA:
      if (!temTec(civ, TC_FERROVIA)) return "É preciso o Caminho de Ferro.";
      if (!(m & M_ESTRADA)) return "Primeiro é preciso uma estrada.";
      if (m & M_FERROVIA) return "Já há caminho de ferro.";
      return nullptr;
    case TB_REGA: {
      if (I.rega == 0 && I.regaMuda < 0) return "Este terreno não se pode regar.";
      if (m & M_REGA) return "Já está regado.";
      if (I.regaMuda >= 0) return nullptr;      // regar transforma (ex.: floresta em planície)
      // é preciso água ao lado: mar, rio, ou outra casa regada
      if (t == T_RIO) return nullptr;
      for (int d = 0; d < 8; d += 2) {
        int nx = x + DIR_DX[d], ny = y + DIR_DY[d];
        if (!dentroY(ny)) continue;
        uint8_t tv = terreno(nx, ny);
        if (tv == T_OCEANO || tv == T_RIO || (melh(nx, ny) & M_REGA) || haCidadeEm(nx, ny)) return nullptr;
      }
      return "Não há água ao lado (mar, rio ou rega).";
    }
    case TB_MINA:
      if (I.mina == 0 && I.minaMuda < 0) return "Neste terreno não se faz mina.";
      if (m & M_MINA) return "Já há mina.";
      return nullptr;
    case TB_FORTALEZA:
      if (!temTec(civ, TC_CONSTRUCAO)) return "É preciso Construção.";
      if (m & M_FORTALEZA) return "Já há fortaleza.";
      return nullptr;
    case TB_LIMPAR:
      if (!(m & M_POLUICAO)) return "Não há poluição aqui.";
      return nullptr;
  }
  return "?";
}

void acabaTrabalho(int i) {
  Unidade & u = U[i];
  int p = idxMapa(u.x, u.y);
  uint8_t t = mapaTer[p] & 15;
  const InfoTerreno & I = TERRENO[t];
  switch (u.trabalho) {
    case TB_ESTRADA:   mapaMelh[p] |= M_ESTRADA; break;
    case TB_FERROVIA:  mapaMelh[p] |= M_FERROVIA; break;
    case TB_REGA:
      if (I.regaMuda >= 0) { mapaTer[p] = (mapaTer[p] & ~15) | I.regaMuda; mapaMelh[p] &= ~(M_REGA | M_MINA); }
      else { mapaMelh[p] |= M_REGA; mapaMelh[p] &= ~M_MINA; }
      break;
    case TB_MINA:
      if (I.minaMuda >= 0) { mapaTer[p] = (mapaTer[p] & ~15) | I.minaMuda; mapaMelh[p] &= ~(M_REGA | M_MINA); }
      else { mapaMelh[p] |= M_MINA; mapaMelh[p] &= ~M_REGA; }
      break;
    case TB_FORTALEZA: mapaMelh[p] |= M_FORTALEZA; break;
    case TB_LIMPAR:    mapaMelh[p] &= ~M_POLUICAO; break;
  }
  u.trabalho = TB_NADA; u.prog = 0; u.estado = EU_NADA;
}

// ---------------- aldeias tribais ----------------
void entraAldeia(int i) {
  Unidade & u = U[i];
  int p = idxMapa(u.x, u.y);
  mapaTer[p] &= ~B_ALDEIA;
  int civ = u.civ;
  bool humano = civ == jogador;
  int r = aleat(100);
  bool pertoDeCidade = false;
  for (int k = 0; k < MAX_CIDADES; ++k)
    if (CID[k].viva && CID[k].civ == civ && distancia(CID[k].x, CID[k].y, u.x, u.y) <= 3) pertoDeCidade = true;
  if (r < 30) {
    int g = 25 + aleat(4) * 25;
    CIV[civ].ouro += g;
    if (humano) mensagemEm(civ, u.x, u.y, "Encontraste ouro numa aldeia: %d moedas!", g);
  } else if (r < 55) {
    // pergaminhos antigos: uma tecnologia
    int cand[N_TECS], n = 0;
    for (int t = 0; t < N_TECS; ++t) if (podePesquisar(civ, t)) cand[n++] = t;
    if (n) {
      int t = cand[aleat(n)];
      ganhaTec(civ, t, false);
      if (humano) mensagemEm(civ, u.x, u.y, "Os sábios da aldeia ensinam-te: %s!", TEC[t].nome);
    }
  } else if (r < 75) {
    // uma tribo amiga junta-se a ti
    int tipo = temTec(civ, TC_CAVALARIA) ? UN_CAVALEIROS : temTec(civ, TC_FERRO) ? UN_LEGIAO : UN_CARRO;
    if (aleat(3) == 0) tipo = UN_COLONOS;
    int n = criaUnidade(tipo, civ, u.x, u.y, SEM);
    if (humano && n >= 0) mensagemEm(civ, u.x, u.y, "Uma tribo amiga junta-se a ti: %s.", UNIDADE[tipo].nome);
  } else if (r < 88 && !pertoDeCidade && turno > 10) {
    // bárbaros!
    int nb = 1 + aleat(3);
    for (int k = 0; k < nb; ++k) {
      int d = aleat(8);
      int nx = u.x + DIR_DX[d], ny = u.y + DIR_DY[d];
      if (dentroY(ny) && !eOceano(nx, ny) && civNaCasa(nx, ny) < 0)
        criaUnidade(temTec(civ, TC_FERRO) ? UN_LEGIAO : UN_CARRO, 0, nx, ny, SEM);
    }
    if (humano) mensagemEm(civ, u.x, u.y, "A aldeia estava cheia de bárbaros!");
  } else {
    if (humano) mensagemEm(civ, u.x, u.y, "A aldeia estava abandonada.");
  }
}

// ---------------- cidades: casas trabalhadas ----------------
int casaX(const Cidade & k, int r) { return embrulhaX(k.x + RAIO_DX[r]); }
int casaY(const Cidade & k, int r) { return k.y + RAIO_DY[r]; }

// a casa (x, y) já é trabalhada por outra cidade?
bool casaOcupada(int x, int y, int exceto) {
  x = embrulhaX(x);
  for (int i = 0; i < MAX_CIDADES; ++i) {
    if (!CID[i].viva || i == exceto) continue;
    const Cidade & k = CID[i];
    if (distancia(k.x, k.y, x, y) > 2) continue;
    if (k.x == x && k.y == y) return true;
    for (int r = 1; r < 21; ++r) if ((k.trab >> r & 1) && casaX(k, r) == x && casaY(k, r) == y) return true;
  }
  return false;
}

// pode a cidade trabalhar a casa r? (vista, sem inimigos, livre)
bool casaDisponivel(int ci, int r) {
  const Cidade & k = CID[ci];
  int x = casaX(k, r), y = casaY(k, r);
  if (!dentroY(y)) return false;
  if (k.civ > 0 && !viu(k.civ, x, y)) return false;
  int dono = civNaCasa(x, y);
  if (dono >= 0 && dono != k.civ) return false;
  return !casaOcupada(x, y, ci);
}

int nTrab(const Cidade & k) { int n = 0; for (int r = 1; r < 21; ++r) if (k.trab >> r & 1) ++n; return n; }
int nEspecialistas(const Cidade & k) { return k.especialistas[0] + k.especialistas[1] + k.especialistas[2]; }

int valorCasa(int ci, int r, bool precisaComida) {
  const Cidade & k = CID[ci];
  Rend v = rendimentoCasa(k.civ, casaX(k, r), casaY(k, r), false);
  return v.comida * (precisaComida ? 5 : 3) + v.escudos * 2 + v.comercio;
}

// arruma os trabalhadores: o número de casas + especialistas tem de dar o tamanho
void arrumaCidade(int ci) {
  Cidade & k = CID[ci];
  // tira casas que deixaram de estar disponíveis
  for (int r = 1; r < 21; ++r) if ((k.trab >> r & 1) && !casaDisponivel(ci, r)) {
    // casaDisponivel vê a própria cidade como livre (exceto = ci), por isso só sai se houver inimigos ou nevoeiro
    k.trab &= ~(1UL << r);
  }
  while (nTrab(k) + nEspecialistas(k) > k.tam) {
    if (nEspecialistas(k) > 0) {
      for (int s = 0; s < 3; ++s) if (k.especialistas[s]) { --k.especialistas[s]; break; }
    } else {
      // larga a pior casa
      int pior = -1, pv = 9999;
      for (int r = 1; r < 21; ++r) if (k.trab >> r & 1) { int v = valorCasa(ci, r, false); if (v < pv) { pv = v; pior = r; } }
      if (pior < 0) break;
      k.trab &= ~(1UL << pior);
    }
  }
  while (nTrab(k) + nEspecialistas(k) < k.tam) {
    // a melhor casa livre; comida primeiro enquanto a cidade não tiver folga
    int comida = 0;
    Rend c0 = rendimentoCasa(k.civ, k.x, k.y, true); comida += c0.comida;
    for (int r = 1; r < 21; ++r) if (k.trab >> r & 1) comida += rendimentoCasa(k.civ, casaX(k, r), casaY(k, r), false).comida;
    bool precisa = comida < k.tam * 2 + 2;
    int melhor = -1, mv = -1;
    for (int r = 1; r < 21; ++r) {
      if (k.trab >> r & 1) continue;
      if (!casaDisponivel(ci, r)) continue;
      int v = valorCasa(ci, r, precisa);
      if (v > mv) { mv = v; melhor = r; }
    }
    if (melhor < 0 || mv <= 0) { ++k.especialistas[0]; continue; }   // sem casa: artista
    k.trab |= 1UL << melhor;
  }
}

// ---------------- cidades: contas ----------------
struct RendCidade {
  int comida, escudos, comercio;      // totais das casas
  int consumo, sobraComida;
  int apoio, sobraEscudos;            // escudos gastos a sustentar unidades
  int corrupcao;
  int luxo, ouro, ciencia;
  int felizes, contentes, descontentes;
  int colonos;                        // colonos sustentados (comem)
  bool desordem;
};

int distanciaCapital(const Cidade & k) {
  int cap = CIV[k.civ].capital;
  if (cap == SEM || !CID[cap].viva || CID[cap].civ != k.civ) return 32;
  return distancia(cap == SEM ? 0 : CID[cap].x, CID[cap].y, k.x, k.y);
}

int contentesBase(int civ) {
  // quantos cidadãos ficam contentes sem ajuda (desce com a dificuldade e com o tamanho do império)
  int b = 4 - dificuldade / 2;
  int limite = 12 - dificuldade * 2;
  if (CIV[civ].humano && contaCidades(civ) > limite) b -= (contaCidades(civ) - limite) / max(4, limite) + 1;
  return max(1, b);
}

RendCidade calculaCidade(int ci) {
  Cidade & k = CID[ci];
  RendCidade r;
  memset(&r, 0, sizeof(r));
  int civ = k.civ, g = CIV[civ].governo;
  bool colosso = cidadeTemMaravilha(ci, W_COLOSSO), ricardo = cidadeTemMaravilha(ci, W_REI_RICARDO);
  for (int q = 0; q < 21; ++q) {
    if (q > 0 && !(k.trab >> q & 1)) continue;
    int x = casaX(k, q), y = casaY(k, q);
    Rend v = rendimentoCasa(civ, x, y, q == 0);
    if (eOceano(x, y)) {
      if (temEdif(k, E_PORTO)) ++v.comida;
      if (colosso) ++v.comercio;
    }
    if (ricardo) ++v.escudos;
    r.comida += v.comida; r.escudos += v.escudos; r.comercio += v.comercio;
  }
  // rotas de comércio abertas pelas caravanas
  r.comercio += k.rotaCom * (2 + k.tam / 4);
  // fábricas e centrais
  int bonus = 0;
  if (temEdif(k, E_FABRICA)) bonus += 50;
  if (temEdif(k, E_FABRICA_ROBOT)) bonus += 50;
  if (bonus && (temEdif(k, E_CENTRAL) || temEdif(k, E_BARRAGEM) || temEdif(k, E_CENTRAL_NUCLEAR) || temMaravilha(civ, W_HOOVER))) bonus += 25;
  r.escudos += r.escudos * bonus / 100;
  // unidades sustentadas pela cidade
  int gratis = (g == G_REPUBLICA || g == G_DEMOCRACIA) ? 0 : 3;
  int militaresFora = 0, emCasa = 0;
  for (int i = 0; i < MAX_UNIDADES; ++i) {
    const Unidade & u = U[i];
    if (!u.viva || u.casa != ci) continue;
    if (UNIDADE[u.tipo].flags & U_COLONO) { ++r.colonos; }
    if (gratis > 0) --gratis; else ++r.apoio;
    if (eMilitar(u.tipo)) {
      int cc = cidadeEm(u.x, u.y);
      if (cc < 0 || CID[cc].civ != civ) ++militaresFora;
    }
  }
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].civ == civ && U[i].x == k.x && U[i].y == k.y && eMilitar(U[i].tipo)) ++emCasa;
  r.consumo = k.tam * 2 + r.colonos * ((g == G_REPUBLICA || g == G_DEMOCRACIA) ? 2 : 1);
  r.sobraComida = r.comida - r.consumo;
  r.sobraEscudos = r.escudos - r.apoio;
  // corrupção (depende da distância à capital e do governo)
  int pct = 0;
  switch (g) {
    case G_ANARQUIA:   pct = 30 + distanciaCapital(k) * 3; break;
    case G_DESPOTISMO: pct = 10 + distanciaCapital(k) * 3; break;
    case G_MONARQUIA:  pct = 5 + distanciaCapital(k) * 2; break;
    case G_COMUNISMO:  pct = 15; break;
    case G_REPUBLICA:  pct = distanciaCapital(k) * 3 / 2; break;
    case G_DEMOCRACIA: pct = 0; break;
  }
  if (temEdif(k, E_PALACIO)) pct = 0;
  if (temEdif(k, E_TRIBUNAL)) pct /= 2;
  pct = min(pct, 90);
  r.corrupcao = r.comercio * pct / 100;
  int com = r.comercio - r.corrupcao;
  // divisão: luxo, impostos, ciência
  if (g == G_ANARQUIA) { r.luxo = 0; r.ouro = 0; r.ciencia = 0; r.luxo = com; }
  else {
    r.luxo = com * CIV[civ].luxo / 10;
    r.ouro = com * CIV[civ].taxa / 10;
    r.ciencia = com - r.luxo - r.ouro;
  }
  r.luxo += k.especialistas[0] * 2;
  r.ouro += k.especialistas[1] * 2;
  r.ciencia += k.especialistas[2] * 2;
  int pctOuro = 100, pctCien = 100;
  if (temEdif(k, E_MERCADO)) { pctOuro += 50; r.luxo += r.luxo / 2; }
  if (temEdif(k, E_BANCO))   { pctOuro += 50; r.luxo += r.luxo / 2; }
  if (temEdif(k, E_BIBLIOTECA)) pctCien += 50;
  if (temEdif(k, E_UNIVERSIDADE)) pctCien += 50;
  if (cidadeTemMaravilha(ci, W_COPERNICO)) pctCien += 100;
  if (cidadeTemMaravilha(ci, W_NEWTON)) pctCien += 100;
  r.ouro = r.ouro * pctOuro / 100;
  r.ciencia = r.ciencia * pctCien / 100;
  // felicidade
  int trab = k.tam - nEspecialistas(k);
  int base = civ == 0 ? 4 : contentesBase(civ);
  r.contentes = min(base, trab);
  r.descontentes = trab - r.contentes;
  r.felizes = 0;
  // luxo: cada 2 tornam um cidadão mais feliz
  for (int l = r.luxo; l >= 2; l -= 2) {
    if (r.contentes > 0) { --r.contentes; ++r.felizes; }
    else if (r.descontentes > 0) { --r.descontentes; ++r.contentes; }
  }
  // edifícios e maravilhas
  int acalma = 0;
  if (temEdif(k, E_TEMPLO)) acalma += temMaravilha(civ, W_ORACULO) ? 4 : 2;
  if (temEdif(k, E_COLISEU)) acalma += 3;
  if (temEdif(k, E_CATEDRAL) || temMaravilha(civ, W_MIGUEL_ANGELO)) acalma += 3;
  if (temMaravilha(civ, W_JARDINS)) acalma += 1;
  if (temMaravilha(civ, W_CANCRO)) acalma += 1;
  if (temMaravilha(civ, W_BACH)) acalma += 2;
  // lei marcial
  if (g == G_MONARQUIA || g == G_COMUNISMO || g == G_DESPOTISMO) acalma += min(3, emCasa);
  int m = min(acalma, r.descontentes);
  r.descontentes -= m; r.contentes += m;
  // tropas fora de casa na república e na democracia
  if (g == G_REPUBLICA || g == G_DEMOCRACIA) {
    int fora = militaresFora * (g == G_DEMOCRACIA ? 2 : 1);
    if (temMaravilha(civ, W_SUFRAGIO)) fora /= 2;
    while (fora-- > 0) {
      if (r.contentes > 0) { --r.contentes; ++r.descontentes; }
      else if (r.felizes > 0) { --r.felizes; ++r.descontentes; }
    }
  }
  if (cidadeTemMaravilha(ci, W_SHAKESPEARE)) { r.contentes += r.descontentes; r.descontentes = 0; }
  r.desordem = r.descontentes > r.felizes;
  if (r.desordem) { r.sobraEscudos = min(0, r.sobraEscudos); r.ouro = 0; r.ciencia = 0; r.luxo = 0; }
  return r;
}

// ---------------- produção ----------------
int custoProd(int p) {
  if (p < PROD_EDIF) return UNIDADE[p].custo;
  if (p < PROD_MARAV) return EDIFICIO[p - PROD_EDIF].custo;
  return MARAVILHA[p - PROD_MARAV].custo;
}
const char * nomeProd(int p) {
  if (p < PROD_EDIF) return UNIDADE[p].nome;
  if (p < PROD_MARAV) return EDIFICIO[p - PROD_EDIF].nome;
  return MARAVILHA[p - PROD_MARAV].nome;
}

bool podeProduzir(int ci, int p) {
  const Cidade & k = CID[ci];
  int civ = k.civ;
  if (p < PROD_EDIF) {
    if (p >= N_UNIDADES) return false;
    const InfoUnidade & I = UNIDADE[p];
    if (!temTec(civ, I.tec)) return false;
    if (I.obsoleta != SEM_TEC && temTec(civ, I.obsoleta)) return false;
    if (I.dominio == D_MAR && !juntoAoMar(k.x, k.y)) return false;
    if (p == UN_NUCLEAR) {
      bool manhattan = false;
      for (int c = 1; c < N_CIVS; ++c) if (temMaravilha(c, W_MANHATTAN)) manhattan = true;
      if (!manhattan) return false;
    }
    return true;
  }
  if (p < PROD_MARAV) {
    int e = p - PROD_EDIF;
    if (e >= N_EDIFICIOS_TOTAL) return false;
    if (!temTec(civ, EDIFICIO[e].tec)) return false;
    if (e >= N_EDIFICIOS) {   // peças da nave: só com o Programa Apollo construído (por alguém)
      bool apollo = false;
      for (int c = 1; c < N_CIVS; ++c) if (temMaravilha(c, W_APOLLO)) apollo = true;
      return apollo && CIV[civ].naveChega == 0 && k.civ == civ && CIV[civ].capital != SEM;
    }
    if (temEdif(k, e)) return false;
    if (e == E_PALACIO && CIV[civ].capital == ci) return false;
    if (e == E_PORTO && !juntoAoMar(k.x, k.y)) return false;
    if (e == E_BARRAGEM) {   // precisa de rio ou montanhas por perto
      bool ok = false;
      for (int r = 0; r < 21; ++r) { uint8_t t = terreno(casaX(k, r), casaY(k, r)); if (t == T_RIO || t == T_MONTANHAS) ok = true; }
      if (!ok) return false;
    }
    if ((e == E_UNIVERSIDADE) && !temEdif(k, E_BIBLIOTECA)) return false;
    if ((e == E_BANCO) && !temEdif(k, E_MERCADO)) return false;
    if ((e == E_FABRICA_ROBOT) && !temEdif(k, E_FABRICA)) return false;
    return true;
  }
  int w = p - PROD_MARAV;
  if (w >= N_MARAVILHAS) return false;
  if (maravilhaCidade[w] >= 0) return false;
  return temTec(civ, MARAVILHA[w].tec);
}

// ---------------- fundar e conquistar cidades ----------------
const char * motivoFundar(int civ, int x, int y) {
  if (eOceano(x, y)) return "Não se fundam cidades no mar.";
  if (terreno(x, y) == T_ARTICO) return "Nem no gelo do Ártico.";
  for (int i = 0; i < MAX_CIDADES; ++i) if (CID[i].viva && distancia(CID[i].x, CID[i].y, x, y) < 2) return "Demasiado perto de outra cidade.";
  return nullptr;
}

// já há uma cidade (viva) com este nome?
bool nomeCidadeUsado(const char * nome) {
  for (int i = 0; i < MAX_CIDADES; ++i) if (CID[i].viva && strcmp(CID[i].nome, nome) == 0) return true;
  return false;
}
// nome sugerido para uma cidade nova: primeiro os nomes do próprio povo que ainda não existem;
// depois os dos povos que não estão em jogo; só no fim se numera (o jogador pode sempre mudar o nome)
void nomeNovaCidade(int civ, char * b) {
  int n = CIV[civ].cidadesFundadas;
  if (civ == 0) { sprintf(b, "Bárbara %d", n + 1); return; }
  const int meu = CIV[civ].povo;
  for (int k = 0; k < N_NOMES_CIDADE; ++k)
    if (!nomeCidadeUsado(POVO[meu].cidades[k])) { strcpy(b, POVO[meu].cidades[k]); return; }
  for (int d = 1; d < N_POVOS; ++d) {
    int p = (meu + d) % N_POVOS;
    bool emJogo = false;
    for (int c = 1; c < N_CIVS; ++c) if (CIV[c].viva && CIV[c].povo == p) emJogo = true;
    if (emJogo) continue;
    for (int k = 0; k < N_NOMES_CIDADE; ++k)
      if (!nomeCidadeUsado(POVO[p].cidades[k])) { strcpy(b, POVO[p].cidades[k]); return; }
  }
  for (int k = 2; k < 99; ++k) {
    snprintf(b, sizeof(((Cidade *) 0)->nome), "%s %d", POVO[meu].cidades[n % N_NOMES_CIDADE], k);
    if (!nomeCidadeUsado(b)) return;
  }
}

int fundaCidade(int civ, int x, int y) {
  for (int i = 0; i < MAX_CIDADES; ++i) {
    if (CID[i].viva) continue;
    Cidade & k = CID[i];
    memset(&k, 0, sizeof(k));
    k.viva = 1; k.civ = k.civOrig = civ; k.x = embrulhaX(x); k.y = y; k.tam = 1;
    nomeNovaCidade(civ, k.nome);
    ++CIV[civ].cidadesFundadas;
    k.prod = UN_MILICIA;
    if (CIV[civ].capital == SEM) { CIV[civ].capital = i; k.edif |= 1UL << E_PALACIO; k.fundada |= 1; }
    int p = idxMapa(x, y);
    mapaMelh[p] |= M_ESTRADA;
    mapaMelh[p] &= ~(M_MINA | M_FORTALEZA);
    mapaTer[p] &= ~B_ALDEIA;
    if (civ > 0) revela(civ, x, y, 2);
    arrumaCidade(i);
    return i;
  }
  return -1;
}

void escolheProducaoIA(int ci);

void capturaCidade(int ci, int novo) {
  Cidade & k = CID[ci];
  int antigo = k.civ;
  k.fundada &= ~CID_AUTO;
  // ouro saqueado: uma parte do tesouro do vencido
  int saque = 0;
  int nc = contaCidades(antigo);
  if (antigo > 0 && nc > 0) { saque = CIV[antigo].ouro / max(1, nc); CIV[antigo].ouro -= saque; CIV[novo].ouro += saque; }
  if (CIV[antigo].capital == ci) {
    CIV[antigo].capital = SEM;
    k.edif &= ~(1UL << E_PALACIO);
  }
  // alguns edifícios ficam destruídos
  for (int e = 0; e < N_EDIFICIOS; ++e) if ((k.edif >> e & 1) && aleat(3) == 0) k.edif &= ~(1UL << e);
  if (k.tam > 1) --k.tam;
  k.civ = novo;
  k.trab = 0;
  k.especialistas[0] = k.especialistas[1] = k.especialistas[2] = 0;
  k.escudos = 0; k.desordem = 0;
  // unidades sustentadas pela cidade ficam sem casa
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].casa == ci) {
    if (U[i].civ == antigo) U[i].casa = SEM;
  }
  if (CIV[novo].capital == SEM && novo > 0) { CIV[novo].capital = ci; k.edif |= 1UL << E_PALACIO; }
  // roubar uma tecnologia que o vencido tinha (como no original)
  if (antigo > 0 && novo > 0) {
    for (int t = 0; t < N_TECS; ++t) if (temTec(antigo, t) && !temTec(novo, t) && podePesquisar(novo, t)) {
      ganhaTec(novo, t, false);
      if (novo == jogador) mensagem("Os sábios de %s ensinam-nos: %s.", k.nome, TEC[t].nome);
      break;
    }
  }
  arrumaCidade(ci);
  if (novo != jogador) escolheProducaoIA(ci);
  else k.prod = UN_MILICIA;
  if (novo > 0) revela(novo, k.x, k.y, 2);
  if (antigo == jogador) mensagemEm(novo, k.x, k.y, "Os %s conquistaram %s!", nomePovo(novo), k.nome);
  else if (novo == jogador) { mensagemEm(novo, k.x, k.y, "Conquistámos %s! Saque: %d moedas.", k.nome, saque); pedeCena(CN_CONQUISTA, ci, saque, corDe(antigo)); }
}

void destroiCidade(int ci) {
  for (int i = 0; i < MAX_UNIDADES; ++i) if (U[i].viva && U[i].casa == ci) U[i].casa = SEM;
  int civ = CID[ci].civ;
  if (CIV[civ].capital == ci) CIV[civ].capital = SEM;
  for (int w = 0; w < N_MARAVILHAS; ++w) if (maravilhaCidade[w] == ci) maravilhaCidade[w] = -2;   // perdida para sempre
  CID[ci].viva = 0;
}

// ---------------- governo ----------------
void mudaGoverno(int civ, int g) {
  if (temMaravilha(civ, W_PIRAMIDES)) { CIV[civ].governo = g; CIV[civ].anarquia = 0; return; }
  CIV[civ].governo = G_ANARQUIA;
  CIV[civ].anarquia = 2;
  CIV[civ].govDepois = g;
}

void ajustaTaxas(int civ) {
  int mx = GOVERNO[CIV[civ].governo].taxaMax;
  Civ & C = CIV[civ];
  if (mx == 0) { C.taxa = 0; C.luxo = 0; return; }
  if (C.taxa > mx) C.taxa = mx;
  if (C.luxo > mx) C.luxo = mx;
  if (10 - C.taxa - C.luxo > mx) C.taxa = 10 - C.luxo - mx;
  if (C.taxa + C.luxo > 10) C.luxo = 10 - C.taxa;
}

// ---------------- pontuação (como no original: cidadãos, maravilhas, tecnologia) ----------------
int calculaPontos(int civ) {
  int p = 0;
  for (int i = 0; i < MAX_CIDADES; ++i) {
    if (!CID[i].viva || CID[i].civ != civ) continue;
    RendCidade r = calculaCidade(i);
    p += r.felizes * 2 + r.contentes;
  }
  for (int w = 0; w < N_MARAVILHAS; ++w) if (maravilhaCidade[w] >= 0 && CID[maravilhaCidade[w]].viva && CID[maravilhaCidade[w]].civ == civ) p += 20;
  p += CIV[civ].nTecs * 2;
  return p;
}
