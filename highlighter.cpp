
#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
  HighlightingRule rule;

  // keywords
  keywordFormat.setForeground(Qt::darkBlue);
  keywordFormat.setFontWeight(QFont::Bold);

  QVector<QString> keywords = {"t", "x", "const", "let", "rpn", "algebraic"};

  for (const QString &pattern : keywords) {
    auto patt = QStringLiteral("\\b") + pattern + QStringLiteral("\\b");
    rule.pattern = QRegularExpression(patt);
    rule.format = keywordFormat;
    highlightingRules.append(rule);
  }

  // symbols
  QVector<QPair<QString, QColor>> sym_cols = {
      {"{}", Qt::red},       {"∿", Qt::red},     {"[]", Qt::blue},
      {"||", Qt::blue},      {"\\\\", Qt::blue}, {"<>", Qt::magenta},
      {"()", Qt::darkGreen},
  };

  for (auto &sc : sym_cols) {
    symbolFormat.setForeground(sc.second);
    auto patt = QStringLiteral("[");
    for (auto &ch : sc.first) patt += QStringLiteral("\\") + ch;
    patt += QStringLiteral("]");
    rule.pattern = QRegularExpression(patt);
    rule.format = symbolFormat;
    highlightingRules.append(rule);
  }

  // comments
  {
    commentFormat.setForeground(Qt::darkCyan);
    rule.pattern = QRegularExpression(QStringLiteral("^//.*"));
    rule.format = commentFormat;
    highlightingRules.append(rule);
  }
}

void Highlighter::highlightBlock(const QString &text) {
  for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
    QRegularExpressionMatchIterator matchIterator =
        rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}
