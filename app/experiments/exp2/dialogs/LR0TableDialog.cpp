#include "LR0TableDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

LR0TableDialog::LR0TableDialog(const LR0Graph& gr, QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("LR(0) DFA 表"));
    auto h     = new QHBoxLayout(this);
    auto left  = new QVBoxLayout;
    auto right = new QVBoxLayout;
    auto lbl1  = new QLabel(QStringLiteral("状态项集"));
    left->addWidget(lbl1);
    auto tblStates = new QTableWidget;
    tblStates->setColumnCount(2);
    tblStates->setHorizontalHeaderLabels(QStringList() << "状态" << "项");
    int rows = 0;
    for (int i = 0; i < gr.states.size(); ++i) rows += gr.states[i].size();
    tblStates->setRowCount(rows);
    int r = 0;
    for (int i = 0; i < gr.states.size(); ++i)
    {
        for (const auto& it : gr.states[i])
        {
            QString rhs;
            for (int k = 0; k < it.right.size(); ++k)
            {
                if (k == it.dot)
                    rhs += " •";
                rhs += " " + it.right[k];
            }
            if (it.dot == it.right.size())
                rhs += " •";
            QString itemText = it.left + " →" + rhs;
            tblStates->setItem(r, 0, new QTableWidgetItem(QString::number(i)));
            tblStates->setItem(r, 1, new QTableWidgetItem(itemText));
            ++r;
        }
    }
    tblStates->setEditTriggers(QAbstractItemView::NoEditTriggers);
    left->addWidget(tblStates);
    auto lbl2 = new QLabel(QStringLiteral("迁移边"));
    right->addWidget(lbl2);
    auto tblEdges = new QTableWidget;
    tblEdges->setColumnCount(3);
    tblEdges->setHorizontalHeaderLabels(QStringList() << "源状态" << "符号" << "目标状态");
    int erows = 0;
    for (auto eit = gr.edges.begin(); eit != gr.edges.end(); ++eit) erows += eit.value().size();
    tblEdges->setRowCount(erows);
    int rr = 0;
    for (auto eit = gr.edges.begin(); eit != gr.edges.end(); ++eit)
    {
        int from = eit.key();
        for (auto sit = eit.value().begin(); sit != eit.value().end(); ++sit)
        {
            tblEdges->setItem(rr, 0, new QTableWidgetItem(QString::number(from)));
            tblEdges->setItem(rr, 1, new QTableWidgetItem(sit.key()));
            tblEdges->setItem(rr, 2, new QTableWidgetItem(QString::number(sit.value())));
            ++rr;
        }
    }
    tblEdges->setEditTriggers(QAbstractItemView::NoEditTriggers);
    right->addWidget(tblEdges);
    h->addLayout(left);
    h->addLayout(right);
    resize(900, 700);
}
