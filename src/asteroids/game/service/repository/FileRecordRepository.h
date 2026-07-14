#pragma once

#include <string>

#include "RecordRepository.h"

namespace ast {

// Persistencia em TSV (uma linha por recorde: nome, pontos, onda, data) — o
// mesmo formato dos jogos irmaos: legivel, editavel e sem dependencia nenhuma.
//
// Falhar em ler ou escrever NAO derruba o jogo: sem arquivo, a lista nasce
// vazia; sem permissao de escrita, o recorde se perde e a partida segue. Placar
// nao vale um crash.
class FileRecordRepository final: public RecordRepository
{
    std::string m_filePath;

public:
    explicit FileRecordRepository(std::string filePath);

    [[nodiscard]] std::vector<Record> loadAll() override;
    void                              saveAll(const std::vector<Record>& records) override;
};

} // namespace ast
