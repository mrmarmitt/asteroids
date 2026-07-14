# 04 - Vidas, pontuacao e gameOver

- **Status:** done (2026-07-14 — validado jogando pelo usuario)
- **Prioridade:** alta - e o que transforma a arena numa PARTIDA (comeco, meio
  e fim).
- **Categoria:** Dominio + Fluxo
- **Depende de:** task 03 (rochas + colisao).

## Contexto

Depois da task 03 o jogo tem nave, rochas e colisao — mas nao tem partida: bater
numa rocha so incrementava um contador (`shipHits()`), o minimo para PROVAR que
a colisao acontecia sem inventar politica. Agora a politica entra.

## Objetivo

Uma partida com comeco, meio e fim: pontuar por rocha destruida, perder vidas ao
bater, e o gameOver quando as vidas acabam — com a tela de resultado e a opcao
de jogar de novo.

## Escopo

1. `ast::World`: `score()`, `lives()`, `outcome()` (Playing/GameOver),
   `shipAlive()`. `shipHits()` foi APOSENTADO — era andaime da task 03.
   - Pontos por rocha destruida **a tiro** (valores do arcade): grande 20, media
     50, pequena 100 — quanto menor, mais dificil de acertar, mais vale.
   - **Bater na rocha NAO pontua** (parte a rocha, mas premiar a batida seria
     absurdo).
   - Vida extra a cada `kExtraLifeScore` (10000) pontos — o "bonus ship".
   - Sem vidas: `Outcome::GameOver`, a nave sai da arena e a simulacao CONGELA
     (nada de ondas novas nascendo atras da tela de game over).
2. Fluxo: `gameOver` entra no `StateGameFlow`/`GameRouter` e no `StateGame`:
   `game -> gameOver -> menu`, e `gameOver -> game` (jogar de novo).
3. `PlaySession` (mesmo padrao do spaceinvaders): o World morre junto com a cena
   do jogo, entao o RESULTADO precisa de alguem que sobreviva a troca de estado.
   Guarda score + onda; nao guarda o World.
4. `ForgeGameOverScene`: pontuacao, onda alcancada, ENTER joga de novo / ESC
   volta ao menu. `ForgeGameScene`: HUD (pontos, vidas em naves, onda) e o
   roteamento ao constatar o gameOver.

## Fora do Escopo

- **Recordes** (task 05): persistir o placar e politica do jogo (TSV, padrao dos
  irmaos) — NAO promove para a engine (ADR 0002).
- Disco voador, som.
- Renderizacao de verdade (decisao ainda adiada — ver task 02).

## Decisoes

**O World constata; o fluxo decide.** O `World` nao conhece cena nem router: ele
apenas expoe `outcome() == GameOver`. Quem le isso, publica o resultado no
`PlaySession` e roteia e a CENA. A alternativa (o World chamar o router) mistura
simulacao com navegacao e mata a testabilidade — a suite dirige o World sem
router nenhum.

**Partida nova nasce zerada de graca.** As factories do `SceneRepository` sao
lazy e recriam a cena a cada visita, e o `World` e membro da cena. Entao
"ENTER para jogar de novo" nao precisa de nenhum `reset()`: a cena velha morre
com o placar dentro, e a nova nasce limpa. O `PlaySession` existe justamente
porque o resultado precisa ATRAVESSAR essa morte.

## Criterios de Aceite

- [x] Suite CMake verde (38 testes; 6 novos: placar inicial, pontos por
      tamanho, batida nao pontua, fim das vidas encerra, gameOver congela a
      simulacao, vida extra no limiar).
- [x] Build MSBuild verde (Release|x64), sem warnings.
- [x] `shipHits()` removido do World (o andaime da task 03 saiu).
- [x] Validacao jogavel: HUD mostra pontos/vidas/onda; morrer 3x leva ao game
      over com o placar certo; ENTER joga de novo (partida zerada) e ESC volta
      ao menu. (Validado em 2026-07-14.)
