# Plano de trabalho - Asteroids

Terceiro jogo do ecossistema cengine. Alem de ser um jogo novo, ele e o
consumidor de validacao da cengine 0.6.0 e do platform-theforge-common 0.1.0
(os jogos anteriores foram congelados — ADR 0003 da cengine).

## Indice

| # | Task | Status | Categoria |
|---|------|--------|-----------|
| 01 | [Bootstrap do casco](01-bootstrap-casco.md) | done | Plataforma |
| 02 | [Dominio: World da nave](02-dominio-world-da-nave.md) | done | Dominio |
| 03 | [Asteroides + colisao](03-asteroides-e-colisao.md) | done | Dominio |
| 04 | [Vidas, pontuacao e gameOver](04-vidas-pontuacao-gameover.md) | in-progress | Dominio + Fluxo |

## Backlog (tasks a abrir quando chegarem)

Sequencia prevista, um degrau validado por vez (mesma disciplina dos jogos
anteriores):

- **05 — Recordes** (politica do jogo, padrao TSV dos irmaos — NAO promove
  para a engine, ADR 0002). O `PlaySession` da task 04 ja carrega o resultado
  que o Record vai consumir.
- **Renderizacao de verdade** (plataforma; a decisao esta madura — o jogo todo
  ja existe e desenha por texto): escolher entre **rotacao no `forgesprite`** e
  um **batcher de linhas (wireframe)** no platform-theforge-common. O arcade
  original e wireframe, e wireframe dispensa atlas de arte (decisao adiada na
  task 02, com o porque).

## Regra pratica

Dominio em `src/asteroids/` sem nenhum include do The-Forge; plataforma em
`src/platform/theforge/`. Toda mexida na cengine ou no common e validada por
este jogo antes de taggear.
