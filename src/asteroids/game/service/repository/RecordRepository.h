#pragma once

#include <vector>

#include "asteroids/game/Record.h"

namespace ast {

// A porta de persistencia dos recordes. O RecordService (as REGRAS) nao sabe se
// isso e arquivo, banco ou memoria — e por isso a suite consegue exercitar as
// regras com um repositorio de mentira, sem tocar em disco.
class RecordRepository
{
public:
    virtual ~RecordRepository() = default;

    [[nodiscard]] virtual std::vector<Record> loadAll() = 0;
    virtual void                              saveAll(const std::vector<Record>& records) = 0;
};

} // namespace ast
