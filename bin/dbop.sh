#!/bin/bash
set -e
set -u

usage(){
    help_info="
    Usage: dbop.sh [option] [operation]\n
    option:\n
    -a ['PROGRAM;PARAM1 PARAM2 ...']\t--  add a new task to the database (task queue) & default state is ready.\n
    -d [ID]\t--  delete a specific task in the db.\n
    -c\t\t--  clean out finished or failed tasks in db file.\n
    -l\t\t--  list all the tasks in the db file.\n"
    echo -e $help_info;
    exit 1;
}

if [[ $# < 1 ]] ; then
    usage
fi

if [ ! -f tasks.db ] ; then
    echo "[init] database nonexistent detected."
    sqlite3 tasks.db "CREATE TABLE tasks (
        ID INTEGER PRIMARY KEY AUTOINCREMENT,
        program VARCHAR(4096),
        param VARCHAR(4096),
        state INTEGER DEFAULT 1
        );"
    sqlite3 tasks.db ".headers on"
    if [[ $? == 1 ]] ; then
        echo "[init] init database error."
        exit 1
    else
        echo "[init] database inited successfully."
    fi
fi

while getopts "lca:d:" opt; do
    case "${opt}" in
        l)
            echo -e " ID | PROGRAM | PARAM | STATE "
            echo    "----+---------+-------+-------"
            sqlite3 tasks.db "SELECT * FROM tasks;" | sed 's/|1/|ready/g' | sed 's/|0/|finished/g' | sed 's/|-1/|failed/g' | sed 's/|/ | /g'
            ;;
        c)
            sqlite3 tasks.db "DELETE FROM tasks WHERE state=0 OR state=-1"
            echo "[clean] clean finished."
            ;;
        a)
            COMMAND=${OPTARG}
            scolon=`expr index "${COMMAND}" ";"`
            PARAMS=${COMMAND:${scolon}}
            POS=$[ ${scolon} - 1 ]
            PROGRAM=`expr substr "${COMMAND}" 1 ${POS}`
            sqlite3 tasks.db "INSERT INTO tasks (ID, program, param, state) VALUES (NULL, '${PROGRAM}', '${PARAMS}', 1);"
            ;;
        d)
            ID=${OPTARG}
            sqlite3 tasks.db "DELETE FROM tasks WHERE ID=${ID}"
            ;;
        *)
            usage
            ;;
    esac
done

