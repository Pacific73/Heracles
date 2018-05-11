#include "db_driver.h"
#include "helpers.h"
#include <cassert>
#include <sqlite3.h>

DatabaseDriver::DatabaseDriver() {
    path = get_opt<std::string>("HERACLES_DB_PATH", "tasks.db");
    print_log("[DBDRIVER] inited.");
}

std::string DatabaseDriver::next_command() {
    sqlite3 *db;
    int res;
    std::string command = "";

    res = sqlite3_open(path.c_str(), &db);
    if (res) {
        print_err("[DBDRIVER] db open failed.");
        exit(-1);
    }

    std::string sql = "SELECT * FROM tasks WHERE state=1 LIMIT 1;";

    char **result;
    char *errmsg = 0;
    int row, col;

    res = sqlite3_get_table(db, sql.c_str(), &result, &row, &col, &errmsg);
    if (res != SQLITE_OK) {
        sqlite3_close(db);
        std::string msg = std::string("[DBDRIVER] SQL select error: ") + errmsg;
        print_err(msg.c_str());
        sqlite3_free(errmsg);
        return command;
    }

    assert(col == 0 || col == 3);
    assert(row == 0 || row == 1);

    if (row == 1) {
        command = result[4];
    }

    sqlite3_close(db);
    return command;
}

void DatabaseDriver::task_finish() {
    sqlite3 *db;
    int res;
    char *errmsg = 0;

    res = sqlite3_open(path.c_str(), &db);
    if (res) {
        print_err("[DBDRIVER] db open failed.");
        exit(-1);
    }

    std::string sql = "UPDATE tasks SET state=0 WHERE state=1 LIMIT 1;";

    res = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errmsg);
    if (res != SQLITE_OK) {
        sqlite3_close(db);
        std::string msg = std::string("[DBDRIVER] SQL update error: ") + errmsg;
        print_err(msg.c_str());
        sqlite3_free(errmsg);
    }

    sqlite3_close(db);
}