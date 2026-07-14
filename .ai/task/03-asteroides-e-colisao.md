# 03 - Asteroides + colisao

- **Status:** in-progress (suite verde; falta a validacao jogavel)
- **Prioridade:** alta - e o jogo virando jogo: sem rochas, so ha uma nave
  passeando.
- **Categoria:** Dominio
- **Depende de:** task 02 (World da nave).
- **Dispara:** o gate da task 17 da cengine (`cengine::collision2d`) — avaliado
  e REPROVADO; ver Decisoes.

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
2. Colisao **circulo x circulo com distancia TOROIDAL**: tiro x rocha (parte a
   rocha, consome o tiro) e nave x rocha (parte a rocha, devolve a nave ao
   centro com invulnerabilidade temporaria).
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

**O gate da cengine (task 17) foi avaliado e REPROVADO — a colisao fica no
jogo.** O criterio 2 da ADR 0002 exige >= 2 consumidores reais, e o candidato
falha por dois motivos que so apareceram com o consumidor na mao:

1. O consumidor que justificava a promocao **nao e mais um consumidor**: o
   spaceinvaders esta congelado na 0.5.0 (ADR 0003) e nunca vai linkar o
   modulo. A evidencia dele e historica, nao viva.
2. A forma que ele evidencia (**AABB**) **nao serve aqui**: rocha e nave sao
   redondas, e a arena e um toro — o asteroids precisa de **circulo com
   distancia toroidal**, que tem zero evidencia previa. Promove-lo seria a
   especulacao que a ADR 0002 proibe.

Registrado na propria task 17 da cengine, com um gate novo e mais afiado:
comecar quando um segundo jogo **VIVO** precisar de deteccao 2D.

Consequencia pratica: `toroidalDelta`/`circlesOverlap` sao publicos e estaticos
no `World` — geometria pura, coberta por testes proprios. E exatamente o codigo
que um dia subiria para a engine, isolado e pronto para ser extraido quando o
gate finalmente passar.

**Colisao continua na nave protegida.** A rocha que mata tambem se parte. Sem
isso, a nave renasceria no centro e a mesma rocha grande viria em cima dela de
novo — uma morte em cascata que o jogador nao tem como evitar.

## Criterios de Aceite

- [x] Suite CMake verde (32 testes; 13 novos: geometria toroidal, fragmentacao
      dos tres tamanhos, tiro consumido no impacto, fragmentos na posicao da
      mae, ondas crescendo com teto, invulnerabilidade e respawn).
- [x] Build MSBuild verde (Release|x64), sem warnings.
- [x] `src/asteroids/` sem nenhum include do The-Forge ou da cengine no World.
- [x] Gate da task 17 da cengine avaliado e documentado nos dois repos.
- [ ] Validacao jogavel: as rochas vagam e dao a volta; o tiro as parte em
      cascata; bater numa rocha devolve a nave ao centro piscando; limpar a
      arena traz uma onda maior.
