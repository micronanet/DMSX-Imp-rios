// =====================================================================
// dados.h — IMPÉRIOS: as regras do jogo em tabelas (terrenos, unidades,
// tecnologias, edifícios, maravilhas, governos, povos, calendário).
// Os valores seguem o Civilization I tão perto quanto possível.
// =====================================================================
#pragma once

// ---------------- terrenos ----------------
enum Terreno : uint8_t { T_OCEANO, T_PRADO, T_PLANICIE, T_DESERTO, T_TUNDRA, T_ARTICO, T_FLORESTA,
                         T_SELVA, T_PANTANO, T_COLINAS, T_MONTANHAS, T_RIO, N_TERRENOS };

struct InfoTerreno {
  const char * nome;
  uint8_t comida, escudos, comercio;      // sem nada
  uint8_t eComida, eEscudos, eComercio;   // com o recurso especial
  const char * especial;
  uint8_t mov;                            // pontos de movimento para entrar
  uint8_t defesa;                         // em metades: 2 = normal, 3 = +50%, 4 = +100%, 6 = +200%
  uint8_t rega;                           // comida a mais com rega (0 = não se rega)
  int8_t  regaMuda;                       // regar transforma neste terreno (-1 = não)
  uint8_t mina;                           // escudos a mais com mina
  int8_t  minaMuda;                       // "minar" transforma neste terreno (-1 = não)
  uint8_t estrada;                        // comércio a mais com estrada
  uint8_t tRega, tMina, tEstrada;         // turnos de trabalho de um colono
};

const InfoTerreno TERRENO[N_TERRENOS] = {
  //  nome          c  e  co   especial            mov def rega muda  mina muda est  tR tM tE
  { "Oceano",       1, 0, 2,  3, 0, 2, "Peixe",      1, 2, 0, -1,      0, -1,      0,  0, 0, 0 },
  { "Prado",        2, 0, 0,  2, 1, 0, "Escudo",     1, 2, 1, -1,      0, T_FLORESTA, 1, 5, 10, 2 },
  { "Planície",     1, 1, 0,  3, 1, 0, "Cavalos",    1, 2, 1, -1,      0, T_FLORESTA, 1, 5, 15, 2 },
  { "Deserto",      0, 1, 0,  3, 1, 0, "Oásis",      1, 2, 1, -1,      1, -1,      1,  5, 5, 2 },
  { "Tundra",       1, 0, 0,  3, 0, 0, "Caça",       1, 2, 0, -1,      0, -1,      0,  0, 0, 2 },
  { "Ártico",       0, 0, 0,  2, 0, 0, "Focas",      2, 2, 0, -1,      0, -1,      0,  0, 0, 4 },
  { "Floresta",     1, 2, 0,  3, 2, 0, "Faisões",    2, 3, 0, T_PLANICIE, 0, -1,     0,  5, 0, 4 },
  { "Selva",        1, 0, 0,  1, 0, 4, "Gemas",      2, 3, 0, T_PRADO,  0, T_FLORESTA, 0, 15, 15, 4 },
  { "Pântano",      1, 0, 0,  1, 4, 0, "Turfa",      2, 3, 0, T_PRADO,  0, T_FLORESTA, 0, 15, 15, 4 },
  { "Colinas",      1, 0, 0,  1, 2, 0, "Carvão",     2, 4, 1, -1,      3, -1,      0,  10, 10, 4 },
  { "Montanhas",    0, 1, 0,  0, 1, 6, "Ouro",       3, 6, 0, -1,      1, -1,      0,  0, 10, 6 },
  { "Rio",          2, 0, 1,  2, 0, 4, "Vinho",      1, 3, 1, -1,      0, -1,      0,  5, 0, 4 },
};

// melhoramentos de cada casa (bits)
const uint8_t M_ESTRADA = 1, M_FERROVIA = 2, M_REGA = 4, M_MINA = 8, M_FORTALEZA = 16, M_POLUICAO = 32;

// ---------------- tecnologias ----------------
enum Tec : uint8_t {
  TC_ALFABETO, TC_BRONZE, TC_CERIMONIAS, TC_EQUITACAO, TC_ALVENARIA, TC_OLARIA,
  TC_CODIGO_LEIS, TC_ESCRITA, TC_CARTOGRAFIA, TC_MOEDA, TC_FERRO, TC_RODA,
  TC_MATEMATICA, TC_MISTICISMO, TC_MONARQUIA, TC_LITERACIA, TC_MARINHARIA, TC_CONSTRUCAO,
  TC_COMERCIO, TC_FILOSOFIA, TC_REPUBLICA, TC_ASTRONOMIA, TC_PONTES, TC_ENGENHARIA,
  TC_FEUDALISMO, TC_CAVALARIA, TC_RELIGIAO, TC_MONOTEISMO, TC_UNIVERSIDADE, TC_BANCA,
  TC_MEDICINA, TC_INVENCAO, TC_NAVEGACAO, TC_FISICA, TC_POLVORA, TC_TEOLOGIA,
  TC_ECONOMIA, TC_MAGNETISMO, TC_GRAVIDADE, TC_QUIMICA, TC_METALURGIA, TC_DEMOCRACIA,
  TC_VAPOR, TC_ELETRICIDADE, TC_EXPLOSIVOS, TC_FERROVIA, TC_CONSCRICAO, TC_INDUSTRIA,
  TC_ATOMICA, TC_ELETRONICA, TC_CORPORACAO, TC_ACO, TC_COMUNISMO, TC_REFINACAO,
  TC_COMBUSTAO, TC_AUTOMOVEL, TC_VOO, TC_RADIO, TC_TATICA, TC_MAQUINAS,
  TC_GUERRA_MOVEL, TC_PRODUCAO_MASSA, TC_SINDICATOS, TC_RECICLAGEM, TC_VOO_AVANCADO, TC_FOGUETOES,
  TC_FISSAO, TC_NUCLEAR, TC_MINIATURA, TC_COMPUTADORES, TC_GENETICA, TC_VOO_ESPACIAL,
  TC_PLASTICOS, TC_SUPERCONDUTOR, TC_ROBOTICA, TC_FUSAO,
  N_TECS
};
const uint8_t SEM_TEC = 255;

struct InfoTec { const char * nome; uint8_t pre1, pre2; };

const InfoTec TEC[N_TECS] = {
  { "Alfabeto",            SEM_TEC, SEM_TEC },
  { "Bronze",              SEM_TEC, SEM_TEC },
  { "Cerimónias Fúnebres", SEM_TEC, SEM_TEC },
  { "Equitação",           SEM_TEC, SEM_TEC },
  { "Alvenaria",           SEM_TEC, SEM_TEC },
  { "Olaria",              SEM_TEC, SEM_TEC },
  { "Código de Leis",      TC_ALFABETO, SEM_TEC },
  { "Escrita",             TC_ALFABETO, SEM_TEC },
  { "Cartografia",         TC_ALFABETO, SEM_TEC },
  { "Moeda",               TC_BRONZE, SEM_TEC },
  { "Ferro",               TC_BRONZE, SEM_TEC },
  { "Roda",                TC_EQUITACAO, SEM_TEC },
  { "Matemática",          TC_ALFABETO, TC_ALVENARIA },
  { "Misticismo",          TC_CERIMONIAS, TC_ALFABETO },
  { "Monarquia",           TC_CERIMONIAS, TC_CODIGO_LEIS },
  { "Literacia",           TC_ESCRITA, TC_CODIGO_LEIS },
  { "Marinharia",          TC_OLARIA, TC_CARTOGRAFIA },
  { "Construção",          TC_OLARIA, TC_MOEDA },
  { "Comércio",            TC_CODIGO_LEIS, TC_ESCRITA },
  { "Filosofia",           TC_MISTICISMO, TC_LITERACIA },
  { "República",           TC_CODIGO_LEIS, TC_LITERACIA },
  { "Astronomia",          TC_MISTICISMO, TC_MATEMATICA },
  { "Pontes",              TC_FERRO, TC_CONSTRUCAO },
  { "Engenharia",          TC_RODA, TC_CONSTRUCAO },
  { "Feudalismo",          TC_MONARQUIA, TC_FERRO },
  { "Cavalaria",           TC_FEUDALISMO, TC_EQUITACAO },
  { "Religião",            TC_FILOSOFIA, TC_MONARQUIA },
  { "Monoteísmo",          TC_RELIGIAO, TC_ASTRONOMIA },
  { "Universidade",        TC_ASTRONOMIA, TC_FILOSOFIA },
  { "Banca",               TC_COMERCIO, TC_REPUBLICA },
  { "Medicina",            TC_FILOSOFIA, TC_COMERCIO },
  { "Invenção",            TC_LITERACIA, TC_ENGENHARIA },
  { "Navegação",           TC_MARINHARIA, TC_ASTRONOMIA },
  { "Física",              TC_NAVEGACAO, TC_LITERACIA },
  { "Pólvora",             TC_INVENCAO, TC_FERRO },
  { "Teologia",            TC_MONOTEISMO, TC_FEUDALISMO },
  { "Economia",            TC_BANCA, TC_UNIVERSIDADE },
  { "Magnetismo",          TC_NAVEGACAO, TC_FISICA },
  { "Gravitação",          TC_UNIVERSIDADE, TC_FISICA },
  { "Química",             TC_UNIVERSIDADE, TC_MEDICINA },
  { "Metalurgia",          TC_POLVORA, TC_UNIVERSIDADE },
  { "Democracia",          TC_BANCA, TC_INVENCAO },
  { "Máquina a Vapor",     TC_FISICA, TC_INVENCAO },
  { "Eletricidade",        TC_METALURGIA, TC_MAGNETISMO },
  { "Explosivos",          TC_POLVORA, TC_QUIMICA },
  { "Caminho de Ferro",    TC_PONTES, TC_VAPOR },
  { "Conscrição",          TC_REPUBLICA, TC_METALURGIA },
  { "Industrialização",    TC_FERROVIA, TC_BANCA },
  { "Teoria Atómica",      TC_GRAVIDADE, TC_FISICA },
  { "Eletrónica",          TC_ENGENHARIA, TC_ELETRICIDADE },
  { "Grandes Empresas",    TC_ECONOMIA, TC_INDUSTRIA },
  { "Aço",                 TC_ELETRICIDADE, TC_INDUSTRIA },
  { "Comunismo",           TC_FILOSOFIA, TC_INDUSTRIA },
  { "Refinação",           TC_QUIMICA, TC_CORPORACAO },
  { "Combustão",           TC_REFINACAO, TC_EXPLOSIVOS },
  { "Automóvel",           TC_COMBUSTAO, TC_ACO },
  { "Voo",                 TC_COMBUSTAO, TC_GRAVIDADE },
  { "Rádio",               TC_VOO, TC_ELETRICIDADE },
  { "Tática",              TC_CONSCRICAO, TC_AUTOMOVEL },
  { "Máquinas-Ferramenta", TC_ACO, TC_TATICA },
  { "Guerra Móvel",        TC_AUTOMOVEL, TC_TATICA },
  { "Produção em Massa",   TC_AUTOMOVEL, TC_CORPORACAO },
  { "Sindicatos",          TC_PRODUCAO_MASSA, TC_COMUNISMO },
  { "Reciclagem",          TC_PRODUCAO_MASSA, TC_DEMOCRACIA },
  { "Aviação Avançada",    TC_RADIO, TC_MAQUINAS },
  { "Foguetões",           TC_VOO_AVANCADO, TC_ELETRONICA },
  { "Fissão Nuclear",      TC_PRODUCAO_MASSA, TC_ATOMICA },
  { "Energia Nuclear",     TC_FISSAO, TC_ELETRONICA },
  { "Miniaturização",      TC_MAQUINAS, TC_ELETRONICA },
  { "Computadores",        TC_PRODUCAO_MASSA, TC_MINIATURA },
  { "Engenharia Genética", TC_MEDICINA, TC_CORPORACAO },
  { "Voo Espacial",        TC_COMPUTADORES, TC_FOGUETOES },
  { "Plásticos",           TC_REFINACAO, TC_VOO_ESPACIAL },
  { "Supercondutores",     TC_PLASTICOS, TC_NUCLEAR },
  { "Robótica",            TC_PLASTICOS, TC_COMPUTADORES },
  { "Fusão Nuclear",       TC_NUCLEAR, TC_SUPERCONDUTOR },
};

// ---------------- unidades ----------------
enum Dominio : uint8_t { D_TERRA, D_MAR, D_AR };
// capacidades especiais
const uint8_t U_COLONO = 1, U_DIPLOMATA = 2, U_CARAVANA = 4, U_TODO_TERRENO = 8, U_NUCLEAR = 16,
              U_SUBMARINO = 32, U_PORTA_AVIOES = 64, U_COSTEIRO = 128;

enum TipoUnidade : uint8_t {
  UN_COLONOS, UN_MILICIA, UN_FALANGE, UN_LEGIAO, UN_CARRO, UN_CATAPULTA, UN_CAVALEIROS,
  UN_MOSQUETEIROS, UN_CANHAO, UN_CAVALARIA, UN_ARTILHARIA, UN_FUZILEIROS, UN_ALPINOS,
  UN_MECANIZADA, UN_BLINDADO, UN_CACA, UN_BOMBARDEIRO, UN_NUCLEAR, UN_TRIRREME, UN_CARAVELA,
  UN_FRAGATA, UN_COURACADO, UN_CRUZADOR, UN_ENCOURACADO, UN_SUBMARINO, UN_PORTA_AVIOES,
  UN_TRANSPORTE, UN_CARAVANA, UN_DIPLOMATA, N_UNIDADES
};

struct InfoUnidade {
  const char * nome;
  uint8_t ataque, defesa, mov;
  uint8_t custo;           // em escudos
  uint8_t tec;             // tecnologia que a permite (SEM_TEC = desde o início)
  Dominio dominio;
  uint8_t carga;           // quantas unidades leva (navios e porta-aviões)
  uint8_t combustivel;     // aviões: turnos que aguentam no ar (0 = não é avião)
  uint8_t flags;
  uint8_t obsoleta;        // tecnologia que a torna obsoleta (SEM_TEC = nunca)
};

const InfoUnidade UNIDADE[N_UNIDADES] = {
  //  nome                   A   D   M  custo  tecnologia          dominio carga comb flags                       obsoleta
  { "Colonos",               0,  1,  1,  30, SEM_TEC,           D_TERRA, 0, 0, U_COLONO,                  SEM_TEC },
  { "Milícia",               1,  1,  1,  10, SEM_TEC,           D_TERRA, 0, 0, 0,                         TC_CONSCRICAO },
  { "Falange",               1,  2,  1,  20, TC_BRONZE,         D_TERRA, 0, 0, 0,                         TC_POLVORA },
  { "Legião",                4,  2,  1,  20, TC_FERRO,          D_TERRA, 0, 0, 0,                         TC_CONSCRICAO },
  { "Carro de Guerra",       3,  1,  2,  30, TC_RODA,           D_TERRA, 0, 0, 0,                         TC_CAVALARIA },
  { "Catapulta",             6,  1,  1,  40, TC_MATEMATICA,     D_TERRA, 0, 0, 0,                         TC_METALURGIA },
  { "Cavaleiros",            4,  2,  2,  40, TC_CAVALARIA,      D_TERRA, 0, 0, 0,                         TC_CONSCRICAO },
  { "Mosqueteiros",          3,  3,  1,  30, TC_POLVORA,        D_TERRA, 0, 0, 0,                         TC_CONSCRICAO },
  { "Canhão",                8,  1,  1,  40, TC_METALURGIA,     D_TERRA, 0, 0, 0,                         TC_MAQUINAS },
  { "Cavalaria",             8,  3,  2,  60, TC_CONSCRICAO,     D_TERRA, 0, 0, 0,                         TC_GUERRA_MOVEL },
  { "Artilharia",           10,  2,  2,  60, TC_MAQUINAS,       D_TERRA, 0, 0, 0,                         SEM_TEC },
  { "Fuzileiros",            5,  4,  1,  40, TC_CONSCRICAO,     D_TERRA, 0, 0, 0,                         TC_SINDICATOS },
  { "Tropas Alpinas",        5,  5,  1,  50, TC_TATICA,         D_TERRA, 0, 0, U_TODO_TERRENO,            SEM_TEC },
  { "Infantaria Mecanizada", 6,  6,  3,  50, TC_SINDICATOS,     D_TERRA, 0, 0, 0,                         SEM_TEC },
  { "Blindado",             10,  5,  3,  80, TC_GUERRA_MOVEL,   D_TERRA, 0, 0, 0,                         SEM_TEC },
  { "Caça",                  4,  2, 10,  60, TC_VOO,            D_AR,    0, 1, 0,                         SEM_TEC },
  { "Bombardeiro",          12,  1,  8, 120, TC_VOO_AVANCADO,   D_AR,    0, 2, 0,                         SEM_TEC },
  { "Míssil Nuclear",       99,  0, 16, 160, TC_FOGUETOES,      D_AR,    0, 1, U_NUCLEAR,                 SEM_TEC },
  { "Trirreme",              1,  0,  3,  40, TC_CARTOGRAFIA,    D_MAR,   2, 0, U_COSTEIRO,                TC_NAVEGACAO },
  { "Caravela",              1,  1,  3,  40, TC_NAVEGACAO,      D_MAR,   3, 0, 0,                         TC_MAGNETISMO },
  { "Fragata",               2,  2,  4,  40, TC_MAGNETISMO,     D_MAR,   2, 0, 0,                         TC_ELETRICIDADE },
  { "Couraçado",             4,  4,  4,  60, TC_VAPOR,          D_MAR,   0, 0, 0,                         TC_ACO },
  { "Cruzador",              6,  6,  6,  80, TC_ACO,            D_MAR,   0, 0, 0,                         SEM_TEC },
  { "Encouraçado",          18, 12,  4, 160, TC_AUTOMOVEL,      D_MAR,   0, 0, 0,                         SEM_TEC },
  { "Submarino",             8,  2,  3,  50, TC_PRODUCAO_MASSA, D_MAR,   0, 0, U_SUBMARINO,               SEM_TEC },
  { "Porta-Aviões",          1, 12,  5, 160, TC_VOO_AVANCADO,   D_MAR,   8, 0, U_PORTA_AVIOES,            SEM_TEC },
  { "Transporte",            0,  3,  5,  50, TC_INDUSTRIA,      D_MAR,   8, 0, 0,                         SEM_TEC },
  { "Caravana",              0,  1,  1,  50, TC_COMERCIO,       D_TERRA, 0, 0, U_CARAVANA,                SEM_TEC },
  { "Diplomata",             0,  1,  2,  30, TC_ESCRITA,        D_TERRA, 0, 0, U_DIPLOMATA,               SEM_TEC },
};

// ---------------- edifícios ----------------
enum Edificio : uint8_t {
  E_PALACIO, E_QUARTEL, E_CELEIRO, E_TEMPLO, E_MERCADO, E_BIBLIOTECA, E_TRIBUNAL, E_MURALHAS,
  E_AQUEDUTO, E_COLISEU, E_CATEDRAL, E_UNIVERSIDADE, E_BANCO, E_PORTO, E_FABRICA, E_CENTRAL,
  E_BARRAGEM, E_CENTRAL_NUCLEAR, E_FABRICA_ROBOT, E_METRO, E_RECICLAGEM, E_SDI,
  N_EDIFICIOS,
  // peças da nave espacial (não ficam na cidade: juntam-se à nave da civilização)
  E_NAVE_ESTRUTURA = N_EDIFICIOS, E_NAVE_COMPONENTE, E_NAVE_MODULO, N_EDIFICIOS_TOTAL
};

struct InfoEdificio { const char * nome; uint16_t custo; uint8_t manutencao; uint8_t tec; };

const InfoEdificio EDIFICIO[N_EDIFICIOS_TOTAL] = {
  { "Palácio",              200, 0, TC_ALVENARIA },
  { "Quartel",               40, 1, SEM_TEC },
  { "Celeiro",               60, 1, TC_OLARIA },
  { "Templo",                40, 1, TC_CERIMONIAS },
  { "Mercado",               80, 1, TC_MOEDA },
  { "Biblioteca",            90, 1, TC_ESCRITA },
  { "Tribunal",              60, 1, TC_CODIGO_LEIS },
  { "Muralhas",             120, 2, TC_ALVENARIA },
  { "Aqueduto",             120, 2, TC_CONSTRUCAO },
  { "Coliseu",              100, 4, TC_CONSTRUCAO },
  { "Catedral",             160, 3, TC_RELIGIAO },
  { "Universidade",         160, 3, TC_UNIVERSIDADE },
  { "Banco",                120, 2, TC_BANCA },
  { "Porto",                 60, 1, TC_MARINHARIA },
  { "Fábrica",              200, 4, TC_INDUSTRIA },
  { "Central Elétrica",     160, 4, TC_REFINACAO },
  { "Barragem",             240, 4, TC_ELETRONICA },
  { "Central Nuclear",      160, 2, TC_NUCLEAR },
  { "Fábrica Robotizada",   320, 6, TC_ROBOTICA },
  { "Metropolitano",        120, 4, TC_PRODUCAO_MASSA },
  { "Centro de Reciclagem", 200, 2, TC_RECICLAGEM },
  { "Defesa SDI",           200, 4, TC_SUPERCONDUTOR },
  { "Estrutura da Nave",     80, 0, TC_VOO_ESPACIAL },
  { "Componente da Nave",   160, 0, TC_PLASTICOS },
  { "Módulo da Nave",       320, 0, TC_SUPERCONDUTOR },
};

// ---------------- maravilhas do mundo ----------------
enum Maravilha : uint8_t {
  W_PIRAMIDES, W_COLOSSO, W_BIBLIOTECA, W_MURALHA, W_JARDINS, W_FAROL, W_ORACULO, W_COPERNICO,
  W_REI_RICARDO, W_MARCO_POLO, W_MIGUEL_ANGELO, W_MAGALHAES, W_SHAKESPEARE, W_BACH, W_NEWTON,
  W_ADAM_SMITH, W_DARWIN, W_SUFRAGIO, W_HOOVER, W_MANHATTAN, W_NACOES_UNIDAS, W_APOLLO, W_CANCRO,
  N_MARAVILHAS
};

struct InfoMaravilha { const char * nome; uint16_t custo; uint8_t tec; uint8_t obsoleta; const char * efeito; };

const InfoMaravilha MARAVILHA[N_MARAVILHAS] = {
  { "Pirâmides",                 300, TC_ALVENARIA,     SEM_TEC,      "Muda de governo sem anarquia." },
  { "Colosso",                   200, TC_BRONZE,        TC_ELETRICIDADE, "+1 comércio no oceano da cidade." },
  { "Grande Biblioteca",         300, TC_LITERACIA,     TC_UNIVERSIDADE, "Dá as tecnologias que 2 outros povos já têm." },
  { "Grande Muralha",            300, TC_CONSTRUCAO,    TC_METALURGIA, "Muralhas em todas as cidades." },
  { "Jardins Suspensos",         300, TC_OLARIA,        TC_FERROVIA,  "+1 contente em todas as cidades." },
  { "Farol de Alexandria",       200, TC_CARTOGRAFIA,   TC_MAGNETISMO, "Navios veteranos; trirremes seguras." },
  { "Oráculo",                   300, TC_MISTICISMO,    TC_RELIGIAO,  "Os templos valem o dobro." },
  { "Observatório de Copérnico", 300, TC_ASTRONOMIA,    SEM_TEC,      "+100% de ciência na cidade." },
  { "Cruzada do Rei Ricardo",    300, TC_ENGENHARIA,    TC_INDUSTRIA, "+1 escudo por casa na cidade." },
  { "Viagem de Marco Polo",      200, TC_COMERCIO,      TC_COMUNISMO, "Contacto com todos os povos." },
  { "Capela de Miguel Ângelo",   400, TC_MONOTEISMO,    SEM_TEC,      "Uma catedral em cada cidade." },
  { "Viagem de Fernão de Magalhães", 400, TC_NAVEGACAO, SEM_TEC,      "+2 de movimento aos navios." },
  { "Teatro de Shakespeare",     300, TC_MEDICINA,      SEM_TEC,      "Ninguém fica descontente na cidade." },
  { "Catedral de J. S. Bach",    400, TC_TEOLOGIA,      SEM_TEC,      "-2 descontentes em todas as cidades." },
  { "Colégio de Isaac Newton",   400, TC_GRAVIDADE,     SEM_TEC,      "+100% de ciência na cidade." },
  { "Comércio de Adam Smith",    400, TC_ECONOMIA,      SEM_TEC,      "Paga a manutenção de 1 dos edifícios." },
  { "Viagem de Darwin",          300, TC_FERROVIA,      SEM_TEC,      "Duas tecnologias de graça." },
  { "Sufrágio Feminino",         600, TC_INDUSTRIA,     SEM_TEC,      "Menos descontentes com tropas fora." },
  { "Barragem Hoover",           600, TC_ELETRONICA,    SEM_TEC,      "Uma central elétrica em cada cidade." },
  { "Projeto Manhattan",         600, TC_FISSAO,        SEM_TEC,      "Todos podem fazer mísseis nucleares." },
  { "Nações Unidas",             600, TC_COMPUTADORES,  SEM_TEC,      "Os outros povos aceitam a paz." },
  { "Programa Apollo",           600, TC_VOO_ESPACIAL,  SEM_TEC,      "Mostra o mundo; permite a nave espacial." },
  { "Cura do Cancro",            600, TC_GENETICA,      SEM_TEC,      "+1 contente em todas as cidades." },
};

// ---------------- governos ----------------
enum Governo : uint8_t { G_ANARQUIA, G_DESPOTISMO, G_MONARQUIA, G_COMUNISMO, G_REPUBLICA, G_DEMOCRACIA, N_GOVERNOS };
struct InfoGoverno { const char * nome; uint8_t tec; uint8_t taxaMax; };   // taxaMax em dezenas de %
const InfoGoverno GOVERNO[N_GOVERNOS] = {
  { "Anarquia",    SEM_TEC,       0 },
  { "Despotismo",  SEM_TEC,       6 },
  { "Monarquia",   TC_MONARQUIA,  7 },
  { "Comunismo",   TC_COMUNISMO,  8 },
  { "República",   TC_REPUBLICA,  8 },
  { "Democracia",  TC_DEMOCRACIA, 9 },
};

// ---------------- povos (civilizações) ----------------
// 7 cores (como no original); cada povo tem a sua cor fixa. Os bárbaros são o 0.
struct InfoCor { uint8_t r, g, b, re, ge, be; };
const InfoCor COR_CIV[8] = {
  { 255,  85,  85, 170,   0,   0 },   // 0 bárbaros: vermelho
  { 255, 255, 255, 170, 170, 170 },   // 1 branco
  {  85, 255,  85,   0, 170,   0 },   // 2 verde
  {  85,  85, 255,   0,   0, 170 },   // 3 azul
  { 255, 255,  85, 170, 170,   0 },   // 4 amarelo
  {  85, 255, 255,   0, 170, 170 },   // 5 ciano
  { 255,  85, 255, 170,   0, 170 },   // 6 rosa
  { 255, 170,  85, 170,  85,   0 },   // 7 laranja
};

const int N_NOMES_CIDADE = 14;
struct InfoPovo {
  const char * nome;        // "Portugueses"
  const char * adjetivo;    // "português"
  const char * lider;
  uint8_t cor;              // 1..7
  uint8_t agressivo;        // 0..2 (para a inteligência artificial)
  const char * cidades[N_NOMES_CIDADE];
};

const int N_POVOS = 15;
const InfoPovo POVO[N_POVOS] = {
  { "Portugueses", "português", "D. Afonso Henriques", 3, 1,
    { "Lisboa", "Porto", "Coimbra", "Braga", "Guimarães", "Évora", "Barcelos", "Viseu", "Faro", "Santarém", "Tomar", "Aveiro", "Leiria", "Setúbal" } },
  { "Romanos", "romano", "Júlio César", 1, 2,
    { "Roma", "Cápua", "Óstia", "Pompeia", "Veios", "Ravena", "Milão", "Verona", "Nápoles", "Génova", "Pisa", "Florença", "Brindisi", "Antium" } },
  { "Babilónios", "babilónio", "Hamurabi", 2, 1,
    { "Babilónia", "Lagash", "Nippur", "Ur", "Uruk", "Kish", "Eridu", "Larsa", "Isin", "Sippar", "Akkad", "Assur", "Nínive", "Mari" } },
  { "Alemães", "alemão", "Frederico, o Grande", 3, 2,
    { "Berlim", "Leipzig", "Hamburgo", "Bremen", "Frankfurt", "Bona", "Nuremberga", "Colónia", "Munique", "Dresden", "Hanôver", "Estugarda", "Kiel", "Lübeck" } },
  { "Egípcios", "egípcio", "Ramsés II", 4, 1,
    { "Tebas", "Mênfis", "Heliópolis", "Gizé", "Alexandria", "Assuão", "Abidos", "Sais", "Bubástis", "Tânis", "Edfu", "Karnak", "Luxor", "Kom Ombo" } },
  { "Americanos", "americano", "Abraham Lincoln", 5, 1,
    { "Washington", "Nova Iorque", "Boston", "Filadélfia", "Atlanta", "Chicago", "Búfalo", "S. Luís", "Detroit", "Nova Orleães", "Baltimore", "Denver", "Seattle", "Dallas" } },
  { "Gregos", "grego", "Alexandre, o Grande", 6, 1,
    { "Atenas", "Esparta", "Corinto", "Delfos", "Erétria", "Faros", "Argos", "Mileto", "Tebas", "Olímpia", "Siracusa", "Rodes", "Pela", "Bizâncio" } },
  { "Indianos", "indiano", "Mahatma Gandhi", 7, 0,
    { "Deli", "Bombaim", "Madrasta", "Bangalore", "Calcutá", "Lahore", "Carachi", "Kolhapur", "Jaipur", "Hiderabade", "Benares", "Goa", "Agra", "Patna" } },
  { "Russos", "russo", "Pedro, o Grande", 1, 2,
    { "Moscovo", "S. Petersburgo", "Kiev", "Minsk", "Esmolensco", "Odessa", "Sebastopol", "Tiflis", "Kazan", "Novgorod", "Iecaterimburgo", "Vladimir", "Tula", "Samara" } },
  { "Zulus", "zulu", "Shaka", 2, 2,
    { "Zimbábue", "Ulundi", "Bapedi", "Hlobane", "Isandlwana", "Intombe", "Mpondo", "Ngome", "Swazi", "Tugela", "Umtata", "Umfolozi", "Ibabanago", "Isipezi" } },
  { "Franceses", "francês", "Napoleão", 3, 2,
    { "Paris", "Orleães", "Lyon", "Tours", "Chartres", "Bordéus", "Ruão", "Avinhão", "Marselha", "Grenoble", "Dijon", "Amiens", "Cherburgo", "Toulouse" } },
  { "Aztecas", "asteca", "Montezuma", 4, 2,
    { "Tenochtitlan", "Chiauhtla", "Chapultepec", "Coatepec", "Ayotzinco", "Itzapalapa", "Iztapam", "Mitxcoac", "Tacubaya", "Tecamac", "Tepezinco", "Ticoman", "Tlaxcala", "Xaltocan" } },
  { "Chineses", "chinês", "Qin Shi Huang", 5, 1,
    { "Pequim", "Xangai", "Cantão", "Nanquim", "Tsingtao", "Hangzhou", "Tientsin", "Macau", "Shenyang", "Xian", "Chongqing", "Wuhan", "Harbin", "Fuzhou" } },
  { "Ingleses", "inglês", "Isabel I", 6, 1,
    { "Londres", "Coventry", "Birmingham", "Dover", "Nottingham", "Iorque", "Liverpool", "Brighton", "Oxford", "Reading", "Exeter", "Cambridge", "Hastings", "Cantuária" } },
  { "Mongóis", "mongol", "Gengis Khan", 7, 2,
    { "Samarcanda", "Bucara", "Nixapur", "Karakorum", "Kashgar", "Tabriz", "Aleppo", "Kabul", "Ormuz", "Basra", "Khanbalyk", "Khorasan", "Shangtu", "Kazan" } },
};

// ---------------- dificuldade ----------------
const int N_DIFICULDADES = 5;
const char * const DIFICULDADE[N_DIFICULDADES] = { "Chefe", "Senhor da Guerra", "Príncipe", "Rei", "Imperador" };

// ---------------- calendário ----------------
// 4000 a.C. a 1000 d.C.: 20 anos por turno; até 1500: 10; até 1750: 5; até 1850: 2; até 2100: 1
const int TURNO_FIM = 650;
int anoDoTurno(int t) {
  if (t < 250) return -4000 + t * 20;
  if (t < 300) return 1000 + (t - 250) * 10;
  if (t < 350) return 1500 + (t - 300) * 5;
  if (t < 400) return 1750 + (t - 350) * 2;
  return 1850 + (t - 400);
}
void textoAno(char * b, int t) {
  int a = anoDoTurno(t);
  if (a < 0) sprintf(b, "%d a.C.", -a);
  else sprintf(b, "%d d.C.", a == 0 ? 1 : a);
}
