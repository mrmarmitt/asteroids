# Plano de trabalho - Asteroids

Terceiro jogo do ecossistema cengine. Alem de ser um jogo novo, ele e o
consumidor de validacao da cengine 0.6.0 e do platform-theforge-common 0.1.0
(os jogos anteriores foram congelados — ADR 0003 da cengine).

## Indice

| # | Task | Status | Categoria |
|---|------|--------|-----------|
| 01 | [Bootstrap do casco](01-bootstrap-casco.md) | done | Plataforma |
| 02 | [Dominio: World da nave](02-dominio-world-da-nave.md) | in-progress | Dominio |

## Backlog (tasks a abrir quando chegarem)

Sequencia prevista, um degrau validado por vez (mesma disciplina dos jogos
anteriores):

- **03 — Asteroides + colisao**: fragmentacao (grande -> medio -> pequeno),
  colisao tiro x asteroide e nave x asteroide. Gate da task 17 da cengine
  (`cengine::collision2d`) dispara aqui — decidir AABB x circulo com o
  consumidor na mao.
- **Renderizacao de verdade** (plataforma; entra junto ou logo apos a 03,
  quando a arte necessaria estiver clara): decidir entre **rotacao no
  `forgesprite`** e um **batcher de linhas (wireframe)** no
  platform-theforge-common — o arcade original e wireframe, e wireframe
  dispensa atlas. Ate la a cena desenha com texto (decisao registrada na
  task 02).
- **04 — Pontuacao, vidas e gameOver**: fluxo completo (estado gameOver
  entra na maquina), UI de HUD.
- **05 — Recordes** (politica do jogo, padrao TSV dos irmaos — NAO promove
  para a engine, ADR 0002).

## Regra pratica

Dominio em `src/asteroids/` sem nenhum include do The-Forge; plataforma em
`src/platform/theforge/`. Toda mexida na cengine ou no common e validada por
este jogo antes de taggear.
