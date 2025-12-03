#include <QString>
#include <QMap>
#include <QVector>

QString generateSyntaxParserSource(const QMap<QString, QMap<QString, int>>& table,
                                   const QVector<QString>&                  nonterms,
                                   const QVector<QString>&                  terms,
                                   const QString&                           start)
{
    QString s;
    s += "#include <iostream>\n#include <vector>\n#include <string>\n#include <unordered_map>\n";
    s += "int main() { std::cout << \"syntax parser stub\"; return 0; }\n";
    return s;
}
