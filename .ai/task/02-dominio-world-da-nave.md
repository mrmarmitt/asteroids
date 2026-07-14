# 02 - Dominio: World da nave

- **Status:** done (2026-07-14 — validado jogando pelo usuario)
- **Prioridade:** alta - e o coracao do jogo; tudo (asteroides, colisao,
  pontuacao) pendura no World.
- **Categoria:** Dominio
- **Depende de:** task 01 (casco).

## Contexto

O casco da task 01 movia um glifo direto na cena, sem dominio nenhum. Agora o
jogo ganha simulacao de verdade: `ast::World`, C++ puro, sem The-Forge e sem
cengine — os testes CMake dirigem a simulacao exatamente como a cena faz
(comandos -> update(dt fixo) -> consultas), que e o que torna o dominio
testavel sem GPU.

## Objetivo

A nave do Asteroids com o "sentir" do arcade: gira, acelera na direcao da
proa, desliza por inercia, da a volta pelas bordas (toro) e atira.

## Escopo

1. `src/asteroids/game/World.{h,cpp}`: arena 800x600 (origem no topo-esquerdo,
   Y para baixo; angulo 0 = proa para cima, crescendo no sentido horario),
   rotacao, thrust com inercia + atrito exponencial, saturacao de velocidade,
   wrap-around, tiros (max 4 em voo, cooldown, tempo de vida, herdam a
   velocidade da nave).
2. `CMakeLists.txt` + `test/` (GoogleTest/CTest, padrao dos irmaos):
   `asteroids_lib` linkando cengine 0.6.0 via FetchContent.
3. `ForgeGameScene` vira casca: traduz teclas SEGURADAS em comandos
   (`heldAxis` girar, `Key::Up` acelerar, `Key::Space` atirar) e desenha
   lendo as consultas.

## Fora do Escopo

- **Rotacao no `forgesprite`** — o backlog previa isso como pre-requisito
  desta task. **Revisado:** a decisao de renderizacao foi ADIADA (ver
  Decisoes). O batcher segue DESLIGADO e a nave e desenhada com texto.
- Asteroides, colisao, pontuacao, vidas, gameOver (tasks 03/04).

## Decisoes

**Renderizacao adiada — nao mexemos no common.** O backlog dizia "task 02 exige
rotacao no forgesprite". Ao chegar aqui, o pre-requisito nao se sustentou:
rotacionar sprites e UMA das formas de desenhar o asteroids, e nem e a mais
fiel — o arcade original e WIREFRAME (linhas), nao sprites. Construir rotacao
no batcher agora seria decidir o renderizador antes de ter o consumidor real
na mao, exatamente o que a ADR 0002 da cengine manda evitar (e o common herda
a mesma disciplina).

Entao: o World nasce completo e testado, a cena desenha a nave como um
triangulo de PONTOS rotacionado pelo angulo do World (`drawText` puro) — feio,
mas suficiente para validar rotacao/inercia/wrap de olho — e a escolha entre
**rotacao no forgesprite** x **batcher de linhas (wireframe)** vira uma task
propria, decidida quando os asteroides existirem e a necessidade de desenho
estiver clara.

## Criterios de Aceite

- [x] Suite CMake verde (16 testes: rotacao, thrust, inercia, atrito,
      saturacao, wrap da nave e do tiro, cooldown, teto de tiros, expiracao,
      heranca de velocidade, clamp do eixo, dt nao-positivo).
- [x] Build MSBuild verde (Release|x64) com o World compilado no vcxproj.
- [x] `src/asteroids/` sem nenhum include do The-Forge ou da cengine no World.
- [x] Validacao jogavel: a nave gira, acelera com inercia, da a volta pelas
      bordas e atira; ESC volta ao menu.

Validado jogando em 2026-07-14: o "sentir" dos controles ficou fiel ao arcade
e as constantes do topo do World.h NAO precisaram de ajuste. O wrap dos TIROS
(e nao so o da nave) foi conferido e confirmado como desejado — e o
comportamento do jogo original: da para se acertar pela borda oposta. O tiro
nunca fecha uma volta completa (kShotSpeed x kShotLife da ~70% da largura da
arena antes de expirar).
