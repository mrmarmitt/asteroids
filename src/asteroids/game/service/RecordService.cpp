#include "RecordService.h"

#include <algorithm>
#include <utility>

#include "repository/RecordRepository.h"

namespace ast {

namespace {

void sortAndTrim(std::vector<Record>& records, const size_t max)
{
    std::stable_sort(records.begin(), records.end(),
                     [](const Record& a, const Record& b) { return a.scoresHigherThan(b); });

    if (records.size() > max)
    {
        records.resize(max);
    }
}

} // namespace

RecordService::RecordService(std::shared_ptr<RecordRepository> repository): m_repository(std::move(repository))
{
    m_records = m_repository->loadAll();

    // O arquivo pode vir desordenado (editado a mao) ou grande demais (versao
    // antiga do jogo): normaliza na entrada, e o resto da classe pode confiar
    // no invariante.
    sortAndTrim(m_records, kMaxRecords);
}

void RecordService::addRecord(const Record& record)
{
    m_records.emplace_back(sanitizeName(record.name()), record.score(), record.wave(), record.playedAt());
    sortAndTrim(m_records, kMaxRecords);

    m_repository->saveAll(m_records);
}

bool RecordService::isNewRecord(const int score) const
{
    if (m_records.size() < kMaxRecords)
    {
        return true; // ainda ha vaga na tabela
    }

    return score > m_records.back().score(); // bate o ultimo colocado?
}

std::string RecordService::sanitizeName(const std::string& name)
{
    std::string clean;
    clean.reserve(std::min(name.size(), kMaxNameLength));

    for (const char c : name)
    {
        if (clean.size() >= kMaxNameLength)
        {
            break;
        }
        if (c != '\t' && c != '\n' && c != '\r')
        {
            clean += c;
        }
    }

    return clean.empty() ? "ANONIMO" : clean;
}

} // namespace ast
