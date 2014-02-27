#ifndef CLICOMMANDREMOVE_H
#define CLICOMMANDREMOVE_H

#include "clicommand.h"

class CliCommandRemove : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
        QString shortHelp() const;
        QString fullHelp() const;
        QString usage() const;
};

#endif // CLICOMMANDREMOVE_H
