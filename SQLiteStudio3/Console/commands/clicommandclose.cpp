#include "clicommandclose.h"
#include "cli.h"
#include "db/db.h"
#include "services/dbmanager.h"

void CliCommandClose::execute()
{
    if (!syntax.isArgumentSet(DB_NAME) && !cli->getCurrentDb())
    {
        println(tr("Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.")
                .arg(cmdName("close")).arg(cmdName("use")).arg(cmdName("close")));
        return;
    }

    if (syntax.isArgumentSet(DB_NAME))
    {
        Db* db = DBLIST->getByName(syntax.getArgument(DB_NAME));
        if (db)
        {
            db->close();
            println(tr("Connection to database %1 closed.").arg(db->getName()));
        }
        else
            println(tr("No such database: %1. Use %2 to see list of known databases.").arg(syntax.getArgument(DB_NAME)).arg(cmdName("dblist")));
    }
    else if (cli->getCurrentDb())
    {
        cli->getCurrentDb()->close();
        println(tr("Connection to database %1 closed.").arg(cli->getCurrentDb()->getName()));
    }
}

QString CliCommandClose::shortHelp() const
{
    return tr("closes given (or current) database");
}

QString CliCommandClose::fullHelp() const
{
    return tr(
                "Closes database connection. If the database was already closed, nothing happens. "
                "If <name> is provided, it should be name of the database to close (as printed by %1 command). "
                "The the <name> is not provided, then current working database is closed (see help for %2 for details)."
                ).arg(cmdName("dblist")).arg(cmdName("use"));
}

void CliCommandClose::defineSyntax()
{
    syntax.setName("close");
    syntax.addArgument(DB_NAME, tr("name", "CLI command syntax"), false);
}
