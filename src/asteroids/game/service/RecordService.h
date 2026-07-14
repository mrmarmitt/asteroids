#pragma once

#include <memory>
#include <vector>

#include "asteroids/game/Record.h"

namespace ast {

class RecordRepository;

// As REGRAS de recorde: ranking por pontuacao e corte do top-N. Politica do
// jogo (ADR 0002 — recordes ficam nos jogos, nunca sobem para a engine).
//
// Diferenca proposital em relacao aos irmaos: o corte do top-N acontece TAMBEM
// ao salvar. No spaceinvaders o arquivo guardava todas as partidas ja jogadas e
// so cortava na exibicao — o arquivo crescia para sempre. Aqui, o que nao entra
// no top-N nao e persistido.
class RecordService
{
    std::shared_ptr<RecordRepository> m_repository;
    std::vector<Record>               m_records; // sempre ordenado, sempre <= kMaxRecords

public:
    /// Quantos recordes o jogo guarda (e mostra).
    static constexpr size_t kMaxRecords = 10;

    /// Nome maior que isso e cortado (a tabela tem largura).
    static constexpr size_t kMaxNameLength = 8;

    explicit RecordService(std::shared_ptr<RecordRepository> repository);

    /// Guarda o recorde e persiste — ja ordenado e cortado no top-N.
    void addRecord(const Record& record);

    /// O top-N, do maior para o menor.
    [[nodiscard]] const std::vector<Record>& listByScore() const { return m_records; }

    /// Esta pontuacao entra no top-N? (O que decide se o jogo pede o nome.)
    [[nodiscard]] bool isNewRecord(int score) const;

    /// Tira do nome o que quebraria o TSV (tab/quebra de linha) e corta o
    /// excesso. Nome vazio vira "ANONIMO" — uma linha sem nome e pior que um
    /// nome feio.
    [[nodiscard]] static std::string sanitizeName(const std::string& name);
};

} // namespace ast
