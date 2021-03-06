#include <Storages/System/StorageSystemUserDirectories.h>
#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <Columns/ColumnString.h>
#include <Columns/ColumnsNumber.h>
#include <Interpreters/Context.h>
#include <Access/AccessControlManager.h>
#include <ext/enumerate.h>


namespace DB
{
NamesAndTypesList StorageSystemUserDirectories::getNamesAndTypes()
{
    NamesAndTypesList names_and_types{
        {"name", std::make_shared<DataTypeString>()},
        {"type", std::make_shared<DataTypeString>()},
        {"path", std::make_shared<DataTypeString>()},
        {"readonly", std::make_shared<DataTypeUInt8>()},
        {"precedence", std::make_shared<DataTypeUInt64>()},
    };
    return names_and_types;
}


void StorageSystemUserDirectories::fillData(MutableColumns & res_columns, const Context & context, const SelectQueryInfo &) const
{
    const auto & access_control = context.getAccessControlManager();
    auto storages = access_control.getStorages();

    size_t column_index = 0;
    auto & column_name = assert_cast<ColumnString &>(*res_columns[column_index++]);
    auto & column_type = assert_cast<ColumnString &>(*res_columns[column_index++]);
    auto & column_path = assert_cast<ColumnString &>(*res_columns[column_index++]);
    auto & column_readonly = assert_cast<ColumnUInt8 &>(*res_columns[column_index++]);
    auto & column_precedence = assert_cast<ColumnUInt64 &>(*res_columns[column_index++]);

    auto add_row = [&](const IAccessStorage & storage, size_t precedence)
    {
        const String & name = storage.getStorageName();
        std::string_view type = storage.getStorageType();
        const String & path = storage.getStoragePath();
        bool readonly = storage.isStorageReadOnly();

        column_name.insertData(name.data(), name.length());
        column_type.insertData(type.data(), type.length());
        column_path.insertData(path.data(), path.length());
        column_readonly.insert(readonly);
        column_precedence.insert(precedence);
    };

    for (auto [i, storage] : ext::enumerate(storages))
        add_row(*storage, i + 1);
}

}
