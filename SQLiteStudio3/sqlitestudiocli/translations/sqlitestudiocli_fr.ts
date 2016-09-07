<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr_FR">
<context>
    <name>CLI</name>
    <message>
        <location filename="../cli.cpp" line="98"/>
        <source>Current database: %1</source>
        <translation>Base de données actuelle : %1</translation>
    </message>
    <message>
        <location filename="../cli.cpp" line="100"/>
        <source>No current working database is set.</source>
        <translation>Aucune base de données en cours n’est activée.</translation>
    </message>
    <message>
        <location filename="../cli.cpp" line="102"/>
        <source>Type %1 for help</source>
        <translation>Touche %1 pour l’aide</translation>
    </message>
    <message>
        <location filename="../cli.cpp" line="257"/>
        <source>Database passed in command line parameters (%1) was already on the list under name: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../cli.cpp" line="264"/>
        <source>Could not add database %1 to list.</source>
        <translation>Impossible d’ajouter la base de données %1 à la liste.</translation>
    </message>
    <message>
        <location filename="../cli.cpp" line="290"/>
        <source>closed</source>
        <translation>Fermé</translation>
    </message>
</context>
<context>
    <name>CliCommand</name>
    <message>
        <location filename="../commands/clicommand.cpp" line="107"/>
        <source>Usage: %1%2</source>
        <translation>Uttilisation : %1%2</translation>
    </message>
</context>
<context>
    <name>CliCommandAdd</name>
    <message>
        <location filename="../commands/clicommandadd.cpp" line="9"/>
        <source>Could not add database %1 to list.</source>
        <translation>Impossible d’ajouter le base de données %1 à la liste.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandadd.cpp" line="14"/>
        <source>Database added: %1</source>
        <translation>Base de données ajoutée : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandadd.cpp" line="19"/>
        <source>adds new database to the list</source>
        <translation>Ajoutez la nouvelle base de données à la liste</translation>
    </message>
    <message>
        <location filename="../commands/clicommandadd.cpp" line="24"/>
        <source>Adds given database pointed by &lt;path&gt; with given &lt;name&gt; to list the databases list. The &lt;name&gt; is just a symbolic name that you can later refer to. Just pick any unique name. For list of databases already on the list use %1 command.</source>
        <translation>Ajoutez la base de données pointée par &lt;chemin&gt; nommée &lt;nom&gt; à la liste des baszs de données. Le &lt;nom&gt;est seulement un nom symbolique que vous pourrez y référer. Choississez un nom unique. Pour une base de données figuant dans la liste utilisez la commande %1.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandadd.cpp" line="34"/>
        <source>name</source>
        <comment>CLI command syntax</comment>
        <translation>Nom</translation>
    </message>
    <message>
        <location filename="../commands/clicommandadd.cpp" line="35"/>
        <source>path</source>
        <comment>CLI command syntax</comment>
        <translation>Chemin</translation>
    </message>
</context>
<context>
    <name>CliCommandCd</name>
    <message>
        <location filename="../commands/clicommandcd.cpp" line="10"/>
        <source>Changed directory to: %1</source>
        <translation>Renommer le repertoire en : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandcd.cpp" line="12"/>
        <source>Could not change directory to: %1</source>
        <translation>Impossible de renommer le répertoire en : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandcd.cpp" line="17"/>
        <source>changes current working directory</source>
        <translation>Modifiezle répertoire de travail actuel</translation>
    </message>
    <message>
        <location filename="../commands/clicommandcd.cpp" line="22"/>
        <source>Very similar command to &apos;cd&apos; known from Unix systems and Windows. It requires a &lt;path&gt; argument to be passed, therefore calling %1 will always cause a change of the directory. To learn what&apos;s the current working directory use %2 command and to list contents of the current working directory use %3 command.</source>
        <translation>La commande « cd » est connu du système UNIX et Windows. Elle nécessite le paramètre &lt;chemin&gt; passé avant l’appel %1 qui occasionnera une modification du répertoire. Pour connaitre qu’elle est le répertoire courant utiliser la commande %2 et pour lister le contenu de celui-ci utilisez la commande %3.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandcd.cpp" line="33"/>
        <source>path</source>
        <comment>CLI command syntax</comment>
        <translation>Chemin</translation>
    </message>
</context>
<context>
    <name>CliCommandClose</name>
    <message>
        <location filename="../commands/clicommandclose.cpp" line="10"/>
        <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
        <translation>Impossible d’appeler %1 lorsqu’aucune base de données n’est active. Spécifiez la base de données active avec la commande %2 ou par le nom de la base de données par %3.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandclose.cpp" line="21"/>
        <location filename="../commands/clicommandclose.cpp" line="29"/>
        <source>Connection to database %1 closed.</source>
        <translation>Connexion à la base de données %1 fermée.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandclose.cpp" line="24"/>
        <source>No such database: %1. Use %2 to see list of known databases.</source>
        <translation>Aucune base de données : %1. Utilisez %2 pour avoir la liste des bases de données connues.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandclose.cpp" line="35"/>
        <source>closes given (or current) database</source>
        <translation>Fermeture d’une de données sélectionnées (ou actuelle)</translation>
    </message>
    <message>
        <location filename="../commands/clicommandclose.cpp" line="40"/>
        <source>Closes database connection. If the database was already closed, nothing happens. If &lt;name&gt; is provided, it should be name of the database to close (as printed by %1 command). The the &lt;name&gt; is not provided, then current working database is closed (see help for %2 for details).</source>
        <translation>Fermeture de base de données connectée. . Si la base est déjà fermée, aucune action. Si &lt;name&gt; est fourni, c’est celle ainsi qui sera close (as printed by %1 command). Si &lt;name&gt; n’est pas fourni, la base actuelle est close (voir l’aide %2 pour plus de détails).</translation>
    </message>
    <message>
        <location filename="../commands/clicommandclose.cpp" line="50"/>
        <source>name</source>
        <comment>CLI command syntax</comment>
        <translation>Nom</translation>
    </message>
</context>
<context>
    <name>CliCommandDbList</name>
    <message>
        <location filename="../commands/clicommanddblist.cpp" line="12"/>
        <source>No current working database defined.</source>
        <translation>Aucune base de données actuelle n’est définie.</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddblist.cpp" line="18"/>
        <source>Databases:</source>
        <translation>Base de données :</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddblist.cpp" line="23"/>
        <location filename="../commands/clicommanddblist.cpp" line="34"/>
        <source>Name</source>
        <comment>CLI db name column</comment>
        <translation>Nom</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddblist.cpp" line="31"/>
        <location filename="../commands/clicommanddblist.cpp" line="61"/>
        <source>Open</source>
        <comment>CLI connection state column</comment>
        <translation>Ouvrir</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddblist.cpp" line="31"/>
        <location filename="../commands/clicommanddblist.cpp" line="61"/>
        <source>Closed</source>
        <comment>CLI connection state column</comment>
        <translation>Fermer</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddblist.cpp" line="32"/>
        <location filename="../commands/clicommanddblist.cpp" line="36"/>
        <source>Connection</source>
        <comment>CLI connection state column</comment>
        <translation>Connexion</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddblist.cpp" line="38"/>
        <location filename="../commands/clicommanddblist.cpp" line="45"/>
        <source>Database file path</source>
        <translation>Chemin de la base de données</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddblist.cpp" line="70"/>
        <source>prints list of registered databases</source>
        <translation>Imprimer la liste des bases de données enregistrées</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddblist.cpp" line="75"/>
        <source>Prints list of databases registered in the SQLiteStudio. Each database on the list can be in open or closed state and %1 tells you that. The current working database (aka default database) is also marked on the list with &apos;*&apos; at the start of its name. See help for %2 command to learn about the default database.</source>
        <translation>Imprimez la liste des bases de données enregistrées sous SQLiteStudio. Chaque base se données de la liste peut être ouverte ou close et %1 vous indique lesquellest.La base de données actuelle est aussi marquée dans la liste par « * » en début de nom. Voir l’aide la commande %2 pour en savoir plus sur la base de données actuelle.</translation>
    </message>
</context>
<context>
    <name>CliCommandDesc</name>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="15"/>
        <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
        <translation type="unfinished">Aucune base de données de travail n’est activée.
Appelez la commande %1 pour activer la base de données active.
Appelez %2 pour voir la liste de toutes les bases de données.</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="26"/>
        <source>Database is not open.</source>
        <translation type="unfinished">La base de données n’est pas ouverte.</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="35"/>
        <source>Cannot find table named: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="52"/>
        <source>shows details about the table</source>
        <translation>Affichage des détails de la table</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="63"/>
        <source>table</source>
        <translation>Table</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="70"/>
        <source>Table: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="74"/>
        <source>Column name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="76"/>
        <source>Data type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="80"/>
        <source>Constraints</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="105"/>
        <source>Virtual table: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="109"/>
        <source>Construction arguments:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../commands/clicommanddesc.cpp" line="114"/>
        <source>No construction arguments were passed for this virtual table.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CliCommandDir</name>
    <message>
        <location filename="../commands/clicommanddir.cpp" line="33"/>
        <source>lists directories and files in current working directory</source>
        <translation>Listes des répertoires et fichiers dans le répertoire de travail</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddir.cpp" line="38"/>
        <source>This is very similar to &apos;dir&apos; command known from Windows and &apos;ls&apos; command from Unix systems.

You can pass &lt;pattern&gt; with wildcard characters to filter output.</source>
        <translation>Ceci est très semblable à la commande « dir » de Windows et à la commande de « ls » de systèmes Unix.

You pouvez utiliser les caractères de remplacement &lt;pattern&gt; npour filtrer la sortie.</translation>
    </message>
    <message>
        <location filename="../commands/clicommanddir.cpp" line="49"/>
        <source>pattern</source>
        <translation>Modèle</translation>
    </message>
</context>
<context>
    <name>CliCommandExit</name>
    <message>
        <location filename="../commands/clicommandexit.cpp" line="12"/>
        <source>quits the application</source>
        <translation>Quitter l’application</translation>
    </message>
    <message>
        <location filename="../commands/clicommandexit.cpp" line="17"/>
        <source>Quits the application. Settings are stored in configuration file and will be restored on next startup.</source>
        <translation>Quittez l’apllication. Le paramètrage est stocké dans la configuration et sera restauré au prochain lancement.</translation>
    </message>
</context>
<context>
    <name>CliCommandHelp</name>
    <message>
        <location filename="../commands/clicommandhelp.cpp" line="16"/>
        <source>shows this help message</source>
        <translation>Affichagez l’aide du message</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhelp.cpp" line="21"/>
        <source>Use %1 to learn about certain commands supported by the command line interface (CLI) of the SQLiteStudio.
To see list of supported commands, type %2 without any arguments.

When passing &lt;command&gt; name, you can skip special prefix character (&apos;%3&apos;).

You can always execute any command with exactly single &apos;--help&apos; option to see help for that command. It&apos;s an alternative for typing: %1 &lt;command&gt;.</source>
        <translation>Utilisez %1 pour connaitre certaines commandes supportées par la ligne de commande (CLI) de SQLiteStudio.
Pour voir les commandes supportées, saississez %2 sana arguments.

En utilisant le nom de &lt;command&gt;, vous ajouter le caractère spécial(« %3 »).

Vous pouvez exécuter n’importe quelle commande avec l’option « --help » pour voir l’aide pour cette commande. C’est une alternative à : %1 &lt;commande&gt;.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhelp.cpp" line="33"/>
        <source>command</source>
        <comment>CLI command syntax</comment>
        <translation>CLI syntax de commandes</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhelp.cpp" line="42"/>
        <source>No such command: %1</source>
        <translation>Aucune telle commande : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhelp.cpp" line="43"/>
        <source>Type &apos;%1&apos; for list of available commands.</source>
        <translation>Saisissez « %1 » pour la liste des commandes valides.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhelp.cpp" line="52"/>
        <source>Usage: %1%2</source>
        <translation>Utilisation : %1%2</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhelp.cpp" line="62"/>
        <source>Aliases: %1</source>
        <translation>Pseudomynes : %1</translation>
    </message>
</context>
<context>
    <name>CliCommandHistory</name>
    <message>
        <location filename="../commands/clicommandhistory.cpp" line="23"/>
        <source>Current history limit is set to: %1</source>
        <translation>L’historique actuel est limité à : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhistory.cpp" line="39"/>
        <source>prints history or erases it</source>
        <translation>Imprimez l’historique ou supprimez le</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhistory.cpp" line="44"/>
        <source>When no argument was passed, this command prints command line history. Every history entry is separated with a horizontal line, so multiline entries are easier to read.

When the -c or --clear option is passed, then the history gets erased.
When the -l or --limit option is passed, it sets the new history entries limit. It requires an additional argument saying how many entries do you want the history to be limited to.
Use -ql or --querylimit option to see the current limit value.</source>
        <translation>Lorqu’aucun argument n’est passé,cette commande imprime l’histoirique. Chaque entrée est séparée par une ligne vide, permettant une lecture aisée.

When the -c or --clear option is passed, then the history gets erased.
When the -l or --limit option is passed, it sets the new history entries limit. It requires an additional argument saying how many entries do you want the history to be limited to.
Use -ql or --querylimit option to see the current limit value.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhistory.cpp" line="59"/>
        <source>number</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhistory.cpp" line="66"/>
        <source>Console history erased.</source>
        <translation>Historique effacé.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhistory.cpp" line="75"/>
        <source>Invalid number: %1</source>
        <translation>Nombre invalide : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandhistory.cpp" line="80"/>
        <source>History limit set to %1</source>
        <translation>Historique limité à %1</translation>
    </message>
</context>
<context>
    <name>CliCommandMode</name>
    <message>
        <location filename="../commands/clicommandmode.cpp" line="9"/>
        <source>Current results printing mode: %1</source>
        <translation>Résultats actuels du mode d’ impression : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandmode.cpp" line="16"/>
        <source>Invalid results printing mode: %1</source>
        <translation>Résultats invalides du mode d’ impression : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandmode.cpp" line="21"/>
        <source>New results printing mode: %1</source>
        <translation>Résultats actuels du mode d’ impression : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandmode.cpp" line="26"/>
        <source>tells or changes the query results format</source>
        <translation>Modifie le format du résultat de la requête</translation>
    </message>
    <message>
        <location filename="../commands/clicommandmode.cpp" line="31"/>
        <source>When called without argument, tells the current output format for a query results. When the &lt;mode&gt; is passed, the mode is changed to the given one. Supported modes are:
- CLASSIC - columns are separated by a comma, not aligned,
- FIXED   - columns have equal and fixed width, they always fit into terminal window width, but the data in columns can be cut off,
- COLUMNS - like FIXED, but smarter (do not use with huge result sets, see details below),
- ROW     - each column from the row is displayed in new line, so the full data is displayed.

The CLASSIC mode is recommended if you want to see all the data, but you don&apos;t want to waste lines for each column. Each row will display full data for every column, but this also means, that columns will not be aligned to each other in next rows. The CLASSIC mode also doesn&apos;t respect the width of your terminal (console) window, so if values in columns are wider than the window, the row will be continued in next lines.

The FIXED mode is recommended if you want a readable output and you don&apos;t care about long data values. Columns will be aligned, making the output a nice table. The width of columns is calculated from width of the console window and a number of columns.

The COLUMNS mode is similar to FIXED mode, except it tries to be smart and make columns with shorter values more thin, while columns with longer values get more space. First to shrink are columns with longest headers (so the header names are to be cut off as first), then columns with the longest values are shrinked, up to the moment when all columns fit into terminal window.
ATTENTION! The COLUMNS mode reads all the results from the query at once in order to evaluate column widhts, therefore it is dangerous to use this mode when working with huge result sets. Keep in mind that this mode will load entire result set into memory.

The ROW mode is recommended if you need to see whole values and you don&apos;t expect many rows to be displayed, because this mode displays a line of output per each column, so you&apos;ll get 10 lines for single row with 10 columns, then if you have 10 of such rows, you will get 100 lines of output (+1 extra line per each row, to separate rows from each other).</source>
        <translation>Sans argument, le format de sortie actuel de la requête est utilisé. Avec &lt;mode&gt;c&apos;est un de ces mode qui est utilisé :
- CLASSIC - columns are separated by a comma, not aligned,
- FIXED   - columns have equal and fixed width, they always fit into terminal window width, but the data in columns can be cut off,
- COLUMNS - like FIXED, but smarter (do not use with huge result sets, see details below),
- ROW     - each column from the row is displayed in new line, so the full data is displayed.

The CLASSIC mode is recommended if you want to see all the data, but you don&apos;t want to waste lines for each column. Each row will display full data for every column, but this also means, that columns will not be aligned to each other in next rows. The CLASSIC mode also doesn&apos;t respect the width of your terminal (console) window, so if values in columns are wider than the window, the row will be continued in next lines.

The FIXED mode is recommended if you want a readable output and you don&apos;t care about long data values. Columns will be aligned, making the output a nice table. The width of columns is calculated from width of the console window and a number of columns.

The COLUMNS mode is similar to FIXED mode, except it tries to be smart and make columns with shorter values more thin, while columns with longer values get more space. First to shrink are columns with longest headers (so the header names are to be cut off as first), then columns with the longest values are shrinked, up to the moment when all columns fit into terminal window.
ATTENTION ! The COLUMNS mode reads all the results from the query at once in order to evaluate column widhts, therefore it is dangerous to use this mode when working with huge result sets. Keep in mind that this mode will load entire result set into memory.

The ROW mode is recommended if you need to see whole values and you don&apos;t expect many rows to be displayed, because this mode displays a line of output per each column, so you&apos;ll get 10 lines for single row with 10 columns, then if you have 10 of such rows, you will get 100 lines of output (+1 extra line per each row, to separate rows from each other).</translation>
    </message>
</context>
<context>
    <name>CliCommandNullValue</name>
    <message>
        <location filename="../commands/clicommandnullvalue.cpp" line="9"/>
        <source>Current NULL representation string: %1</source>
        <translation>Représentation actuelle d’une chaine NULL : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandnullvalue.cpp" line="15"/>
        <source>tells or changes the NULL representation string</source>
        <translation>Modifiez la représentation d’une chaine NULL</translation>
    </message>
    <message>
        <location filename="../commands/clicommandnullvalue.cpp" line="20"/>
        <source>If no argument was passed, it tells what&apos;s the current NULL value representation (that is - what is printed in place of NULL values in query results). If the argument is given, then it&apos;s used as a new string to be used for NULL representation.</source>
        <translation>Si on n’a passé aucun argument, c’est la représentation de valeur NULL actuelle qui est utilisée (ce qui est imprimé à la place de valeurs NULL dans des résultats de requête). Si on donne un argument, il sera utilisé comme une nouvelle chaine représentant NULL.</translation>
    </message>
</context>
<context>
    <name>CliCommandOpen</name>
    <message>
        <location filename="../commands/clicommandopen.cpp" line="12"/>
        <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
        <translation>Impossible d’appeler %1 lorsque aucune base de données n’est sélectionnée. Spécifiez la base de données actuelle avec la commande %2 ou nommez la base de données %3.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandopen.cpp" line="29"/>
        <source>Could not add database %1 to list.</source>
        <translation>Impossible d’ajouter la base de données %1 à la liste.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandopen.cpp" line="37"/>
        <source>File %1 doesn&apos;t exist in %2. Cannot open inexisting database with %3 command. To create a new database, use %4 command.</source>
        <translation>Le fichier %1 n’existe pas dans %2. Impossible d’ouvrir une base de données avec la commande %3. Pour créer une nouvelle base de données utilisez la commande %4.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandopen.cpp" line="61"/>
        <source>Database %1 has been open and set as the current working database.</source>
        <translation>La base de données %1 a été ouverte et sélectionnée comme base de données actuelle.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandopen.cpp" line="66"/>
        <source>opens database connection</source>
        <translation>Ouvre la connexion de la base de données</translation>
    </message>
    <message>
        <location filename="../commands/clicommandopen.cpp" line="71"/>
        <source>Opens connection to the database. If no additional argument was passed, then the connection is open to the current default database (see help for %1 for details). However if an argument was passed, it can be either &lt;name&gt; of the registered database to open, or it can be &lt;path&gt; to the database file to open. In the second case, the &lt;path&gt; gets registered on the list with a generated name, but only for the period of current application session. After restarting application such database is not restored on the list.</source>
        <translation>Ouvre la connexion de la base de données. Si aucun argument n’est passé, alors la connexion est ouverte comme base de données actuelle (voir l’aide %1 pour plus de détails). Cependant si on a passé un argument il peut être le &lt;name&gt; d’une base de données enregistrée, ou cela peut être le &lt;chemin&gt; du fichier de base de données. Dans le deuxième cas, le &lt;chemin&gt; est enregistré dans la liste avec un nom généré mais seulement pendant la période de la session actuelle. Après la reprise de la l’application une telle base de données n’est pas rétablie dans la liste.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandopen.cpp" line="83"/>
        <source>name</source>
        <comment>CLI command syntax</comment>
        <translation>Nom</translation>
    </message>
    <message>
        <location filename="../commands/clicommandopen.cpp" line="83"/>
        <source>path</source>
        <comment>CLI command syntax</comment>
        <translation>Chemin</translation>
    </message>
</context>
<context>
    <name>CliCommandPwd</name>
    <message>
        <location filename="../commands/clicommandpwd.cpp" line="13"/>
        <source>prints the current working directory</source>
        <translation>Imprime le répertoire de travail actuel</translation>
    </message>
    <message>
        <location filename="../commands/clicommandpwd.cpp" line="18"/>
        <source>This is the same as &apos;pwd&apos; command on Unix systems and &apos;cd&apos; command without arguments on Windows. It prints current working directory. You can change the current working directory with %1 command and you can also list contents of the current working directory with %2 command.</source>
        <translation>C’est la même commande d’un système Unix « pwd » ou « cd » sans arguments de Windows. Ceci imprimele répertoire de travail courant. Vous pouvez changer le répertoire avec le commande %1 et avoir la liste des répertoire de travail avec la commande %2.</translation>
    </message>
</context>
<context>
    <name>CliCommandRemove</name>
    <message>
        <location filename="../commands/clicommandremove.cpp" line="12"/>
        <source>No such database: %1</source>
        <translation>Aucune base de données : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandremove.cpp" line="20"/>
        <source>Database removed: %1</source>
        <translation>Base de données enlevée : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandremove.cpp" line="26"/>
        <source>New current database set:</source>
        <translation>Nouvelle base de données actuelle :</translation>
    </message>
    <message>
        <location filename="../commands/clicommandremove.cpp" line="35"/>
        <source>removes database from the list</source>
        <translation>Enleve la base de données de la liste</translation>
    </message>
    <message>
        <location filename="../commands/clicommandremove.cpp" line="40"/>
        <source>Removes &lt;name&gt; database from the list of registered databases. If the database was not on the list (see %1 command), then error message is printed and nothing more happens.</source>
        <translation>Enlève la base de données &lt;nom&gt; de la liste des bases enregistrées.si la base de données n’est pas dans la liste (voir la commande %1), alors message d’erreur est imprimé sans aucunes autres lignes.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandremove.cpp" line="50"/>
        <source>name</source>
        <comment>CLI command syntax</comment>
        <translation>Nom</translation>
    </message>
</context>
<context>
    <name>CliCommandSql</name>
    <message>
        <location filename="../commands/clicommandsql.cpp" line="18"/>
        <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
        <translation>Aucune base de données de travail n’est activée.
Appelez la commande %1 pour activer la base de données active.
Appelez %2 pour voir la liste de toutes les bases de données.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandsql.cpp" line="29"/>
        <source>Database is not open.</source>
        <translation>La base de données n’est pas ouverte.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandsql.cpp" line="64"/>
        <source>executes SQL query</source>
        <translation>Exécute la requête SQL</translation>
    </message>
    <message>
        <location filename="../commands/clicommandsql.cpp" line="69"/>
        <source>This command is executed every time you enter SQL query in command prompt. It executes the query on the current working database (see help for %1 for details). There&apos;s no sense in executing this command explicitly. Instead just type the SQL query in the command prompt, without any command prefixed.</source>
        <translation>Cette commande est exécutée chaque fois vous saississez une requête SQL au prompt de commande. Il exécute la requête sur la base de données actuelle (voir l’aide %1 pour les détails). Il n’y a aucun sens dans l’exécution de cette commande explicitement. Instead just type the SQL query in the command prompt, without any command prefixed.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandsql.cpp" line="85"/>
        <source>sql</source>
        <comment>CLI command syntax</comment>
        <translation>SQL</translation>
    </message>
    <message>
        <location filename="../commands/clicommandsql.cpp" line="136"/>
        <location filename="../commands/clicommandsql.cpp" line="178"/>
        <source>Too many columns to display in %1 mode.</source>
        <translation>Trop de colonnes à afficher avec le mode %1.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandsql.cpp" line="256"/>
        <source>Row %1</source>
        <translation>Ligne %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommandsql.cpp" line="406"/>
        <source>Query execution error: %1</source>
        <translation>Erreur d’exécution de la requête : %1</translation>
    </message>
</context>
<context>
    <name>CliCommandTables</name>
    <message>
        <location filename="../commands/clicommandtables.cpp" line="15"/>
        <source>No such database: %1. Use %2 to see list of known databases.</source>
        <translation>Aucune base de données : %1. Utilisez %2pour voir la liste des base de données connues.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtables.cpp" line="25"/>
        <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
        <translation>Impossible d’appeler %1 quand aucune base de données n’est active. Spécifiez la base de données active avec la commade %2 ou nommez la base de données avec %3.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtables.cpp" line="32"/>
        <source>Database %1 is closed.</source>
        <translation>La base de données %1 est fermée.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtables.cpp" line="45"/>
        <location filename="../commands/clicommandtables.cpp" line="47"/>
        <source>Database</source>
        <translation>Base de données</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtables.cpp" line="47"/>
        <source>Table</source>
        <translation>Table</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtables.cpp" line="61"/>
        <source>prints list of tables in the database</source>
        <translation>Imprime la liste des tablesde la base de données</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtables.cpp" line="66"/>
        <source>Prints list of tables in given &lt;database&gt; or in the current working database. Note, that the &lt;database&gt; should be the name of the registered database (see %1). The output list includes all tables from any other databases attached to the queried database.
When the -s option is given, then system tables are also listed.</source>
        <translation>Imprime la liste des tables de la &lt;base de données&gt; sélectionnée ou de la base de données actuelle. Notez que la &lt;base de données&gt; devrait être le nom enregistré de la base de données (voir %1). L’affichage de la liste inclus toutes les tables d’autres bases de données attachées à celle-ci.
Lorsque l’option « -s » est ajouté, les tables système sont aussi listées.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtables.cpp" line="77"/>
        <source>database</source>
        <comment>CLI command syntax</comment>
        <translation>Base de données</translation>
    </message>
</context>
<context>
    <name>CliCommandTree</name>
    <message>
        <location filename="../commands/clicommandtree.cpp" line="12"/>
        <source>No current working database is selected. Use %1 to define one and then run %2.</source>
        <translation>Aucune base de données actuelle n’est sélectionnée. Utilisez %1 pour en définir uneet lancez avec %2.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtree.cpp" line="54"/>
        <source>Tables</source>
        <translation>Tables</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtree.cpp" line="58"/>
        <source>Views</source>
        <translation>Vues</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtree.cpp" line="83"/>
        <source>Columns</source>
        <translation>Colonnes</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtree.cpp" line="88"/>
        <source>Indexes</source>
        <translation>Index</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtree.cpp" line="92"/>
        <location filename="../commands/clicommandtree.cpp" line="113"/>
        <source>Triggers</source>
        <translation>Déclancheurs</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtree.cpp" line="132"/>
        <source>prints all objects in the database as a tree</source>
        <translation>Imprime tous les objets de la base de données comme un arbre</translation>
    </message>
    <message>
        <location filename="../commands/clicommandtree.cpp" line="137"/>
        <source>Prints all objects (tables, indexes, triggers and views) that are in the database as a tree. The tree is very similar to the one that you can see in GUI client of the SQLiteStudio.
When -c option is given, then also columns will be listed under each table.
When -s option is given, then also system objects will be printed (sqlite_* tables, autoincrement indexes, etc).
The database argument is optional and if provided, then only given database will be printed. This is not a registered database name, but instead it&apos;s an internal SQLite database name, like &apos;main&apos;, &apos;temp&apos;, or any attached database name. To print tree for other registered database, call %1 first to switch the working database, and then use %2 command.</source>
        <translation>Imprime tous les objets (tables, index, déclencheurs et vues) qui sont dans la base de données comme un arbre. L’arbre est très semblable à celui que vous pouvez voir dans lGUI client de SQLiteStudio.
Quand on ajoute l’option-c, alors aussi les colonnes seront inscrites sous chaque table.
Quand on ajoute l’option-s, alors aussi les objets de système seront imprimés (sqlite_* tables, des index d’auto-incrément, etc).
L’argument de base de données est facultatif et si fourni, alors seulement la base de données indiquée sera imprimée. Ceci n’est pas un nom de base de données enregistré, mais au lieu de cela c’est un nom de base de données SQLite interne, comme « principal », « temporaire », ou n’importe quel nom de base de données attaché. Pour imprimer l’arbre pour d’autre base de données enregistrée, appelez %1 d’abord pour changer la base de données actuelleet utiliser la commande %2.</translation>
    </message>
</context>
<context>
    <name>CliCommandUse</name>
    <message>
        <location filename="../commands/clicommanduse.cpp" line="13"/>
        <source>No current database selected.</source>
        <translation>Aucune base de données active de sélectionnée.</translation>
    </message>
    <message>
        <location filename="../commands/clicommanduse.cpp" line="16"/>
        <location filename="../commands/clicommanduse.cpp" line="30"/>
        <source>Current database: %1</source>
        <translation>Base de données actuelle : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommanduse.cpp" line="23"/>
        <source>No such database: %1</source>
        <translation>Aucune base de données : %1</translation>
    </message>
    <message>
        <location filename="../commands/clicommanduse.cpp" line="35"/>
        <source>changes default working database</source>
        <translation>Change la base de données actelle par défaut</translation>
    </message>
    <message>
        <location filename="../commands/clicommanduse.cpp" line="40"/>
        <source>Changes current working database to &lt;name&gt;. If the &lt;name&gt; database is not registered in the application, then the error message is printed and no change is made.

What is current working database?
When you type a SQL query to be executed, it is executed on the default database, which is also known as the current working database. Most of database-related commands can also work using default database, if no database was provided in their arguments. The current database is always identified by command line prompt. The default database is always defined (unless there is no database on the list at all).

The default database can be selected in various ways:
- using %1 command,
- by passing database file name to the application startup parameters,
- by passing registered database name to the application startup parameters,
- by restoring previously selected default database from saved configuration,
- or when default database was not selected by any of the above, then first database from the registered databases list becomes the default one.</source>
        <translation>Changet la base de données actuelle &lt;nom&gt;. Si le &lt;nom &gt; de la base de données n’est pas enregistrée dans l’application, le message d’erreur est imprimé et aucun changement n’est fait.

Quel est la base de données actuelle ?
Quand vous saississez une requête SQL à exécuter, celle-ci est exécutée sur la base de données par défaut, que l’on connaît aussi comme la base de données actuelle. La plupart de commandes concernant la base de données utilise la base de données de défaut d’utilisation, si on n’a fourni aucune base de données dans leurs arguments. La base de données actuelle est toujours identifiée par la ligne de commande. La base de données par défaut est toujours définie (à moins qu’il n’y ait aucune base de données dans la liste).

La base de données par défaut peut être choisie de diverses manières :
- Utilisation de la commande %1,
- En passant nom de fichier de base de données aux paramètres de démarrage d’application,
- En passantle nom la base de données enregistrée aux paramètres de démarrage d’application,
- En restaurant la base de données par défaut précédemment choisie dans la configuration sauvée,
- Ou quand la base de données par défaut n’a été choisie par aucun du susdit, l’alors première base de données de la liste de bases de données enregistrée devient le par défaut.</translation>
    </message>
    <message>
        <location filename="../commands/clicommanduse.cpp" line="63"/>
        <source>name</source>
        <comment>CLI command syntax</comment>
        <translation>Nom</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../clicommandsyntax.cpp" line="155"/>
        <source>Insufficient number of arguments.</source>
        <translation>Nombre d’arguments insuffisant.</translation>
    </message>
    <message>
        <location filename="../clicommandsyntax.cpp" line="325"/>
        <source>Too many arguments.</source>
        <translation>Trop d’arguements.</translation>
    </message>
    <message>
        <location filename="../clicommandsyntax.cpp" line="347"/>
        <source>Invalid argument value: %1.
Expected one of: %2</source>
        <translation>Valeur invalide de l’arguement %1. Excepté l’un d’eux : %2</translation>
    </message>
    <message>
        <location filename="../clicommandsyntax.cpp" line="383"/>
        <source>Unknown option: %1</source>
        <comment>CLI command syntax</comment>
        <translation>Option %1 inconnue</translation>
    </message>
    <message>
        <location filename="../clicommandsyntax.cpp" line="394"/>
        <source>Option %1 requires an argument.</source>
        <comment>CLI command syntax</comment>
        <translation>L’option %1 nécessite un argument.</translation>
    </message>
    <message>
        <location filename="../commands/clicommandnullvalue.cpp" line="31"/>
        <source>string</source>
        <comment>CLI command syntax</comment>
        <translation>Chaîne</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="22"/>
        <source>Command line interface to SQLiteStudio, a SQLite manager.</source>
        <translation>Interface de ligne de commandes de SQLiteStudio, SQLite manager.</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="26"/>
        <source>Enables debug messages on standard error output.</source>
        <translation>Messages de débogage valides sur sortie d’erreur standard.</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="27"/>
        <source>Enables Lemon parser debug messages for SQL code assistant.</source>
        <translation>Permet le débogage avec l’analyseur syntaxique de Lemon pour l’assistant SQL.</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="28"/>
        <source>Lists plugins installed in the SQLiteStudio and quits.</source>
        <translation>Liste les plugins installés dans SQLiteStudio et quitte.</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="33"/>
        <source>file</source>
        <translation>Fichier</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="33"/>
        <source>Database file to open</source>
        <translation>Base de données à ouvrir</translation>
    </message>
</context>
</TS>
