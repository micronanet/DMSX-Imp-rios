// =====================================================================
// imperios.ino — IMPÉRIOS
//
// Jogo de estratégia por turnos para o sistema FoBoS (ESP32 + FabGL),
// tão parecido quanto possível com o Civilization I: de 4000 a.C. até
// 2100 d.C., com até 7 povos, cidades, tecnologias, maravilhas, governos,
// guerra, diplomacia e a corrida às estrelas.
//
// Arduino IDE: a pasta tem de se chamar "imperios" e conter imperios.ino e os .h.
// Placa: ESP32 Dev Module (com PSRAM, se houver). FabGL 1.0.9 com o core ESP32 2.0.x.
// Hardware: VGA de 64 cores a 640x200 (pinos 22,21,19,18,5,4 + sync 23,15),
// teclado PS/2 na porta 0, som no GPIO 25, cartão SD no pino 13 (CS) para gravar,
// e, se existir, o MCP23017 dos joysticks em 0x20 (I2C 26/27).
//
// Teclas no mapa (como no original):
//   setas / teclado numérico (7 9 1 3 = diagonais)  mover   ENTER  ordens / fim do turno
//   B fundar cidade   R estrada   I regar   M mina   F fortificar   S sentinela   G ir para
//   W esperar   ESPAÇO sem ordens   H casa   D dispensar   U acordar   C cidade   V ver
//   T impostos   X revolução   Y diplomacia   F1 cidades   F3 povos   F6 ciência
//   F7 maravilhas   F9 pontuação   F10 gravar   ESC menu do jogo
// Joystick: direções (diagonais com duas), botão A = ordens / escolher, botão B = menu / voltar.
// =====================================================================

#include "fabgl.h"
#include <Wire.h>
#include <SD.h>
using namespace fabgl;

fabgl::VGAController DisplayController;
fabgl::Canvas        canvas(&DisplayController);
fabgl::PS2Controller PS2;
SoundGenerator       soundGenerator;

#include "arte.h"
#include "dados.h"
#include "base.h"
#include "estado.h"
#include "mundo.h"
#include "regras.h"
#include "turno.h"
#include "ia.h"
#include "ecras.h"
#include "jogo.h"

void setup() {
  Serial.begin(115200);
  PS2.begin(PS2Preset::KeyboardPort0, KbdMode::CreateVirtualKeysQueue);
  DisplayController.begin(GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18,
                          GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_23, GPIO_NUM_15);
  DisplayController.setResolution(VGA_640x200_70Hz);
  preparaFonte();
  canvas.selectFont(&fonte);
  canvas.setGlyphOptions(GlyphOptions().FillBackground(false));
  randomSeed(esp_random());
  rngEstado = esp_random() | 1;

  if (!reservaMemoria()) {
    canvas.setPenColor(C_VERMELHO);
    canvas.drawText(10, 10, "Sem memoria para o mapa!");
    while (true) delay(1000);
  }
  preparaArte();

  // música: duas vozes sempre ligadas ao gerador de som
  vozMelodia.setVolume(18);
  vozBaixo.setVolume(40);
  soundGenerator.attach(&vozMelodia);
  soundGenerator.attach(&vozBaixo);
  vozMelodia.enable(false);
  vozBaixo.enable(false);
  soundGenerator.play(true);

  // joystick no MCP23017 (0x20), se existir
  Wire.begin(26, 27);
  Wire.setClock(400000);
  Wire.beginTransmission(0x20);
  temJoystick = Wire.endTransmission() == 0;
  if (temJoystick) { Wire.beginTransmission(0x20); Wire.write(0x00); Wire.write(0xFF); Wire.endTransmission(); }

  temSD = SD.begin(13);
  Serial.printf("IMPERIOS: joystick %s, cartao SD %s\n", temJoystick ? "sim" : "nao", temSD ? "sim" : "nao");
  entraTitulo();
}

void loop() {
  static uint32_t ultimo = 0;
  uint32_t agora = millis();
  if (agora - ultimo < 20) return;          // 50 fotogramas por segundo
  ultimo = agora;
  ++frame; ++tempo;
  leEntrada();
  atualizaMusica(agora);
  if (premiu(VK_F12)) musicaPermitida = !musicaPermitida;

  switch (modo) {
    case M_TITULO:    break;
    case M_LISTA:     atualizaListaModo(); break;
    case M_MAPA:      atualizaMapa(); break;
    case M_MENSAGEM:  atualizaMensagem(); break;
    case M_PERGUNTA:  atualizaPergunta(); break;
    case M_CIDADE:    atualizaCidade(); break;
    case M_RELATORIO: atualizaRelatorio(); break;
    case M_DESTINO:   atualizaDestino(); break;
    case M_TAXAS:     atualizaTaxas(); break;
    case M_FIM:       atualizaFim(); break;
    case M_ESPERA:    break;
    case M_ENCONTRO:  atualizaEncontro(); break;
  }
}
