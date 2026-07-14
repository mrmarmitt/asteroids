# 03 - Asteroides + colisao

- **Status:** in-progress (suite verde; falta a validacao jogavel)
- **Prioridade:** alta - e o jogo virando jogo: sem rochas, so ha uma nave
  passeando.
- **Categoria:** Dominio
- **Depende de:** task 02 (World da nave).
- **Dispara:** o gate da task 17 da cengine — que virou o **ciclo 0.7.0**
  (`cengine::collision2d`, AABB + circulo) e a Emenda 1 da ADR 0002; ver
  Decisoes.

## Contexto

A nave da task 02 voa numa arena vazia. Agora entram as rochas: elas vagam,
dao a volta pela borda, se partem quando levam tiro e matam a nave quando a
alcancam. Com isso surge a primeira colisao do jogo — e a pergunta que estava
agendada desde o inicio: a deteccao sobe para a engine?

## Objetivo

Uma arena viva: ondas de rochas grandes que se fragmentam em medias e pequenas
sob tiro, e que devolvem a nave ao centro quando a acertam.

## Escopo

1. `ast::World`: rochas (`AsteroidSize` Large/Medium/Small com raio e
   velocidade proprios — quanto menor, mais rapida), ondas com RNG
   deterministico (LCG por seed, como no spaceinvaders), deriva + wrap.
2. Colisao **circulo x circulo** — deteccao via `cengine::collision2d`, com a
   correcao TOROIDAL feita aqui: tiro x rocha (parte a rocha, consome o tiro) e
   nave x rocha (parte a rocha, devolve a nave ao centro com invulnerabilidade
   temporaria).
3. Fragmentacao: grande -> 2 medias -> 2 pequenas -> some. Fragmentos nascem
   onde a mae morreu, em direcoes sorteadas e mais rapidos.
4. Ondas: arena limpa faz a proxima nascer, com uma rocha grande a mais (com
   teto). Nave nasce protegida (`kSpawnInvulnerability`), e a cena a pisca.
5. Ganchos de teste (`addAsteroid`/`clearAsteroids`, padrao do `killInvader` do
   spaceinvaders): a suite monta a arena que quer, sem depender do sorteio.

## Fora do Escopo

- **Vidas, pontuacao e gameOver** (task 04). Por ora a batida so incrementa
  `shipHits()` — o contador que PROVA que a colisao acontece, sem inventar
  politica de partida.
- Disco voador, gravidade, som.
- Renderizacao de verdade: as rochas sao aneis de pontos de texto. A escolha do
  renderizador continua adiada (ver task 02).

## Decisoes

**A DETECCAO subiu para a cengine 0.7.0; o TORO ficou aqui.**

Historico honesto do gate: numa primeira leitura eu REPROVEI a promocao,
entendendo o criterio 2 da ADR 0002 ("≥ 2 consumidores reais") como "dois jogos
que vao LINKAR o modulo" — e o spaceinvaders, congelado (ADR 0003), nunca vai.
O dono do projeto corrigiu a regra: **congelar um jogo suspende a manutencao
dele, nao o aprendizado que ele produziu**. Sem isso, "documentacao viva" (ADR
0003) seria so abandono, e nenhuma promocao futura passaria — os dois jogos
completos do ecossistema estao parados.

A **Emenda 1 da ADR 0002** oficializou a regra e cobrou um pedagio: a suite da
engine tem de **encarnar o caso de uso do jogo congelado**. Os testes de
`cengine::collision2d` reproduzem tiro x invasor e bomba x canhao do
spaceinvaders — sem descongela-lo.

O recorte, que e o que importa aqui:

- **Sobe (mecanismo):** as formas e a sobreposicao entre elas — `Aabb`,
  `Circle`, `intersects()`. Circulo e AABB sao irmaos; as duas evidencias reais
  existem.
- **Fica (politica):** o **wrap-around**. A arena do asteroids e um toro; a do
  spaceinvaders nao. Formato do mundo e decisao DO JOGO. O `toroidalDelta`
  continua nosso: corrigimos a posicao pelo menor delta e so entao perguntamos
  a engine. Promover o toro daria a cengine uma opiniao sobre o formato do
  mundo — e e assim que uma engine vira deposito.

Na pratica, `World::circlesOverlap` virou tres linhas: acha o ponto equivalente
mais proximo no toro e delega a `cengine::collision2d::intersects`.

**Colisao continua na nave protegida.** A rocha que mata tambem se parte. Sem
isso, a nave renasceria no centro e a mesma rocha grande viria em cima dela de
novo — uma morte em cascata que o jogador nao tem como evitar.

## Criterios de Aceite

- [x] Suite CMake verde (32 testes; 13 novos: geometria toroidal, fragmentacao
      dos tres tamanhos, tiro consumido no impacto, fragmentos na posicao da
      mae, ondas crescendo com teto, invulnerabilidade e respawn).
- [x] Build MSBuild verde (Release|x64), sem warnings.
- [x] `src/asteroids/` sem nenhum include do The-Forge (o World usa a cengine:
      `collision2d` para detectar, e so isso).
- [x] Gate da task 17 avaliado, corrigido pelo dono e executado: cengine 0.7.0
      publicada, com o asteroids consumindo `cengine::collision2d` e o
      wrap-around ficando no jogo.
- [ ] Validacao jogavel: as rochas vagam e dao a volta; o tiro as parte em
      cascata; bater numa rocha devolve a nave ao centro piscando; limpar a
      arena traz uma onda maior.
