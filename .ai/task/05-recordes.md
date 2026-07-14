# 05 - Recordes

- **Status:** done (2026-07-14 — validado jogando pelo usuario)
- **Prioridade:** media - fecha o ciclo da partida (jogar -> perder -> se ver na
  tabela -> querer de novo).
- **Categoria:** Politica do jogo
- **Depende de:** task 04 (o `PlaySession` ja carrega score + onda).

## Contexto

A task 04 deu um fim a partida; falta o que faz querer jogar de novo. Recordes
sao o exemplo canonico da ADR 0002 da cengine: codigo QUASE identico nos tres
jogos e que, mesmo assim, **nao sobe para a engine** — o que e um recorde, qual
metrica ranqueia, quantos guardar e onde persistir sao decisoes DO JOGO. A
duplicacao entre os jogos e o custo aceito (e explicitamente registrado no ADR).

## Objetivo

Top-10 persistido: ao acabar a partida, se a pontuacao entra na tabela, o jogo
pede o nome e grava; a tabela e vista pelo menu ou logo apos gravar.

## Escopo

1. Dominio (`src/asteroids/game/`):
   - `Record` (nome, pontos, onda, data) + `nowAsString()`.
   - `RecordRepository` (porta) e `FileRecordRepository` (TSV, padrao dos
     irmaos: legivel, editavel, sem dependencia).
   - `RecordService` (as REGRAS): ranking por pontos, corte do top-10,
     `isNewRecord(score)`, `sanitizeName()`.
2. Fluxo: `records` entra no `StateGameFlow`/`GameRouter`/`StateGame`.
   `menu -> records`, `gameOver -> records` (depois de gravar), `records -> menu`.
3. Cenas: `ForgeRecordsScene` (a tabela) e a entrada de nome no
   `ForgeGameOverScene` (so quando ha recorde). Menu ganha a opcao RECORDES.

## Fora do Escopo

- Promover qualquer parte disto para a cengine (ADR 0002 — veredito ja dado).
- Recorde online / por dificuldade.
- Renderizacao de verdade (decisao ainda adiada — ver task 02).

## Decisoes

**O corte do top-N acontece AO SALVAR, nao so ao exibir.** Diferenca proposital
em relacao ao spaceinvaders: la o arquivo guardava toda partida ja jogada e o
top-N era so um corte na tela — o arquivo crescia para sempre. Aqui, o que nao
entra na tabela nao e persistido, e o `RecordService` normaliza (ordena + corta)
tambem na LEITURA, para sobreviver a um arquivo editado a mao.

**Nome e higienizado, nao validado.** Um tab no nome partiria a linha do TSV em
campos errados na proxima leitura — entao `sanitizeName()` tira tab/quebra de
linha, corta o excesso e chama de `ANONIMO` quem nao digitou nada. O jogo nunca
recusa o nome do jogador: ele conserta e segue.

**Placar nao vale um crash.** Sem arquivo, a tabela nasce vazia (primeiro uso
NAO e erro). Sem permissao de escrita, o recorde se perde e a partida segue.
Linha corrompida e ignorada e as boas sobrevivem.

**"Isto e recorde?" e pergunta de ATIVACAO.** A decisao mora no `onEnter()` da
cena de game over, nao no construtor: depende do placar que a cena do jogo
acabou de publicar E da tabela como ela esta agora.

## Criterios de Aceite

- [x] Suite CMake verde (53 testes; 15 novos: ranking, corte do top-N, expulsao
      do ultimo colocado, isNewRecord com e sem vaga, normalizacao de arquivo
      torto, higiene do nome; e o TSV de verdade em disco — round-trip, arquivo
      ausente, reescrita, linhas quebradas).
- [x] Build MSBuild verde (Release|x64), sem warnings.
- [x] Regras testadas SEM disco (repositorio de mentira) e o disco testado a
      parte — o `RecordService` nao sabe o que ha do outro lado da porta.
- [x] Validacao jogavel: fazer pontos, morrer, ver NOVO RECORDE, digitar o nome,
      ver a tabela; abrir RECORDES pelo menu; fechar e reabrir o jogo e a tabela
      continuar la. (Validado em 2026-07-14.)
