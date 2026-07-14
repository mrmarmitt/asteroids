# Plano de trabalho - Asteroids

Terceiro jogo do ecossistema cengine. Alem de ser um jogo novo, ele e o
**consumidor de validacao** da cengine e do platform-theforge-common (os jogos
anteriores foram congelados — ADR 0003 da cengine). Ja puxou tres releases:
cengine 0.6.0 (owned/FlowRouter), cengine 0.7.x (collision2d) e common 0.2.0
(forgeline).

## Indice

| # | Task | Status | Categoria |
|---|------|--------|-----------|
| 01 | [Bootstrap do casco](01-bootstrap-casco.md) | done | Plataforma |
| 02 | [Dominio: World da nave](02-dominio-world-da-nave.md) | done | Dominio |
| 03 | [Asteroides + colisao](03-asteroides-e-colisao.md) | done | Dominio |
| 04 | [Vidas, pontuacao e gameOver](04-vidas-pontuacao-gameover.md) | done | Dominio + Fluxo |
| 05 | [Recordes](05-recordes.md) | done | Politica do jogo |
| 06 | [Renderizacao wireframe](06-renderizacao-wireframe.md) | in-progress | Plataforma |

## Backlog

Vazio — o jogo esta completo (casco, nave, rochas, partida, recordes e o
desenho vetorial). O que vier agora e polimento ou escopo novo: disco voador,
som, hyperspace.

## Regra pratica

Dominio em `src/asteroids/` sem nenhum include do The-Forge; plataforma em
`src/platform/theforge/`. Toda mexida na cengine ou no common e validada por
este jogo antes de taggear.
