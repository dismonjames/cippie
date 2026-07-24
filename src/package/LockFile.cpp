#include <cippie/package/LockFile.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace cippie
{
    void LockFile::addPackage(LockedPackage pkg)
    {
        auto it = std::find_if(m_packages.begin(), m_packages.end(), [&](const auto& p) {
            return p.name == pkg.name;
        });
        if (it != m_packages.end())
        {
            *it = std::move(pkg);
        }
        else
        {
            m_packages.push_back(std::move(pkg));
        }
        std::sort(m_packages.begin(), m_packages.end());
    }

    const LockedPackage* LockFile::findPackage(const std::string& name) const
    {
        for (const auto& pkg : m_packages)
        {
            if (pkg.name == name) return &pkg;
        }
        return nullptr;
    }

    Result<LockFile> LockFile::load(const std::filesystem::path& lockFilePath)
    {
        std::ifstream file(lockFilePath);
        if (!file.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to open lock file: " + lockFilePath.string(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        LockFile lf;
        std::string line;
        LockedPackage currentPkg;
        bool inPackage = false;

        while (std::getline(file, line))
        {
            if (line.empty() || line.front() == '#') continue;

            std::stringstream ss(line);
            std::string key;
            ss >> key;

            if (key == "version")
            {
                uint32_t ver = 0;
                ss >> ver;
                if (ver != LOCK_SCHEMA_VERSION)
                {
                    return std::unexpected(Error{
                        .code = ErrorCode::validationFailed,
                        .message = "unsupported lock file schema version: " + std::to_string(ver),
                        .location = std::nullopt,
                        .notes = {}
                    });
                }
            }
            else if (key == "package")
            {
                if (inPackage)
                {
                    lf.addPackage(std::move(currentPkg));
                    currentPkg = {};
                }
                ss >> currentPkg.name;
                inPackage = true;
            }
            else if (key == "version_str")
            {
                std::string verStr;
                ss >> verStr;
                auto vRes = SemanticVersion::parse(verStr);
                if (!vRes.has_value()) return std::unexpected(vRes.error());
                currentPkg.version = *vRes;
            }
            else if (key == "source")
            {
                std::string typeStr;
                ss >> typeStr >> currentPkg.sourceLocation;
                if (typeStr == "path") currentPkg.sourceType = PackageSourceType::path;
                else if (typeStr == "git") currentPkg.sourceType = PackageSourceType::git;
                else currentPkg.sourceType = PackageSourceType::registry;
            }
            else if (key == "commit")
            {
                ss >> currentPkg.commit;
            }
            else if (key == "integrity")
            {
                ss >> currentPkg.integrity;
            }
            else if (key == "dependencies")
            {
                std::string depToken;
                while (ss >> depToken)
                {
                    currentPkg.dependencies.push_back(depToken);
                }
            }
        }

        if (inPackage)
        {
            lf.addPackage(std::move(currentPkg));
        }

        return lf;
    }

    Result<void> LockFile::save(const std::filesystem::path& lockFilePath) const
    {
        std::error_code ec;
        std::filesystem::create_directories(lockFilePath.parent_path(), ec);

        const auto tmpPath = lockFilePath.string() + ".tmp";
        std::ofstream file(tmpPath);
        if (!file.is_open())
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to create lock file: " + tmpPath,
                .location = std::nullopt,
                .notes = {}
            });
        }

        file << "# Cippie Lock File (v" << LOCK_SCHEMA_VERSION << ")\n";
        file << "version " << LOCK_SCHEMA_VERSION << "\n\n";

        auto sortedPkgs = m_packages;
        std::sort(sortedPkgs.begin(), sortedPkgs.end());

        for (const auto& pkg : sortedPkgs)
        {
            file << "package " << pkg.name << "\n";
            file << "  version_str " << pkg.version.toString() << "\n";
            file << "  source ";
            switch (pkg.sourceType)
            {
                case PackageSourceType::path: file << "path "; break;
                case PackageSourceType::git: file << "git "; break;
                case PackageSourceType::registry: file << "registry "; break;
            }
            file << pkg.sourceLocation << "\n";

            if (!pkg.commit.empty()) file << "  commit " << pkg.commit << "\n";
            if (!pkg.integrity.empty()) file << "  integrity " << pkg.integrity << "\n";

            file << "  dependencies";
            for (const auto& dep : pkg.dependencies)
            {
                file << " " << dep;
            }
            file << "\n\n";
        }

        file.flush();
        file.close();

        std::filesystem::rename(tmpPath, lockFilePath, ec);
        if (ec)
        {
            return std::unexpected(Error{
                .code = ErrorCode::fileReadFailed,
                .message = "failed to rename lock file: " + ec.message(),
                .location = std::nullopt,
                .notes = {}
            });
        }

        return {};
    }
}
