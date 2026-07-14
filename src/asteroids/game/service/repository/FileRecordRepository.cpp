#include "FileRecordRepository.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace ast {

namespace {

constexpr char kSeparator = '\t';

// Uma linha TSV -> Record; false se a linha nao tem os 4 campos validos. Linha
// corrompida e IGNORADA (nao aborta a leitura): um arquivo meio estragado ainda
// devolve os recordes que sobraram.
bool parseLine(const std::string& line, Record& out)
{
    std::istringstream in(line);
    std::string        name;
    std::string        score;
    std::string        wave;
    std::string        playedAt;

    if (!std::getline(in, name, kSeparator))
        return false;
    if (!std::getline(in, score, kSeparator))
        return false;
    if (!std::getline(in, wave, kSeparator))
        return false;
    if (!std::getline(in, playedAt))
        return false;

    try
    {
        out = Record(name, std::stoi(score), static_cast<uint32_t>(std::stoul(wave)), playedAt);
    }
    catch (...)
    {
        return false; // numeros invalidos -> linha ignorada
    }
    return true;
}

} // namespace

FileRecordRepository::FileRecordRepository(std::string filePath): m_filePath(std::move(filePath)) {}

std::vector<Record> FileRecordRepository::loadAll()
{
    std::vector<Record> records;

    std::ifstream file(m_filePath);
    if (!file.is_open())
    {
        return records; // primeiro uso: nada persistido ainda
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (Record record; parseLine(line, record))
        {
            records.push_back(record);
        }
    }
    return records;
}

void FileRecordRepository::saveAll(const std::vector<Record>& records)
{
    std::ofstream file(m_filePath, std::ios::trunc);
    if (!file.is_open())
    {
        return; // sem onde salvar (ex.: diretorio sem escrita) — o jogo segue
    }

    for (const auto& record : records)
    {
        file << record.name() << kSeparator << record.score() << kSeparator << record.wave() << kSeparator
             << record.playedAt() << '\n';
    }
}

} // namespace ast
