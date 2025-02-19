// Copyright (c) 2015-2017, The Bytecoin developers
// Copyright (c) 2017-2018, The Karbo developers
// Copyright (c) 2019, The Qwertycoin developers
//
// This file is part of Qwertycoin.
//
// Qwertycoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Qwertycoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Qwertycoin.  If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <QClipboard>
#include <QDataWidgetMapper>

#include "crypto/crypto.h"
#include "CryptoNoteCore/CryptoNoteBasic.h"
#include "Common/StringTools.h"
#include "TransactionDetailsDialog.h"
#include "Settings/Settings.h"
#include "CopyColumnDelegate.h"
#include "Models/TransactionsModel.h"
#include "Models/TransfersModel.h"
#include "RightAlignmentColumnDelegate.h"
#include "TransactionsAmountDelegate.h"
#include "TransfersHeaderView.h"
#include "Style/Style.h"
#include "ui_TransactionDetailsDialog.h"

Q_DECLARE_METATYPE(QList<CryptoNote::WalletTransfer>)

namespace WalletGui {

namespace {

const char TRANSACTION_DETAILS_DIALOG_STYLE_SHEET_TEMPLATE[] =
  "* {"
    "font-family: %fontFamily%;"
  "}"

  "WalletGui--TransactionDetailsDialog {"
    "min-height: 430px;"
    "max-height: 430px;"
  "}"

  "WalletGui--TransactionDetailsDialog WalletGui--WalletNormalGrayTextLabel, "
  "WalletGui--TransactionDetailsDialog WalletGui--WalletNormalBlackTextLabel {"
    "min-height: 41px;"
    "max-height: 41px;"
    "padding-left: 30px;"
    "padding-right: 30px;"
    "border: none;"
  "}"

  "WalletGui--TransactionDetailsDialog #m_textLabel,"
  "WalletGui--TransactionDetailsDialog #m_hashLabel {"
    "padding-left: 0px;"
    "padding-right: 0px;"
  "}"

  "WalletGui--TransactionDetailsDialog #m_statusTextLabel,"
  "WalletGui--TransactionDetailsDialog #m_statusLabel,"
  "WalletGui--TransactionDetailsDialog #m_heightTextLabel,"
  "WalletGui--TransactionDetailsDialog #m_heightLabel,"
  "WalletGui--TransactionDetailsDialog #m_amountTextLabel,"
  "WalletGui--TransactionDetailsDialog #m_amountLabel,"
  "WalletGui--TransactionDetailsDialog #m_paymentIdTextLabel,"
  "WalletGui--TransactionDetailsDialog #m_paymentIdLabel {"
    "background-color: %backgroundColorAlternate%;"
  "}"

  "WalletGui--TransactionDetailsDialog > #m_transactionDetailsTabBar {"
    "border: none;"
  "}"

  "WalletGui--TransactionDetailsDialog > #m_transactionDetailsTabBar::pane {"
    "border: none;"
    "border-top: 1px solid %borderColorDark%;"
    "top: -1px;"
  "}"

  "WalletGui--TransactionDetailsDialog > #m_transactionDetailsTabBar::tab-bar {"
    "alignment: center;"
  "}"

  "WalletGui--TransactionDetailsDialog > #m_transactionDetailsTabBar QTabBar::tab {"
    "font-size: %fontSizeLarge%;"
    "font-weight: bold;"
    "min-width: 300px;"
    "max-width: 300px;"
    "min-height: 30px;"
    "max-height: 30px;"
    "border: 1px solid %borderColorDark%;"
    "margin-left: 2px;"
    "margin-right: 2px;"
    "border-top-left-radius: 2px;"
    "border-top-right-radius: 2px;"
  "}"

  "WalletGui--TransactionDetailsDialog > #m_transactionDetailsTabBar QTabBar::tab:selected {"
    "background: #ffffff;"
    "border: 1px solid %borderColorDark%;"
    "border-bottom: 1px solid #ffffff;"
  "}"

  "WalletGui--TransactionDetailsDialog > #m_transactionDetailsTabBar QTabBar::tab:!selected {"
    "background: %backgroundColorGray%;"
    "border: 1px solid %borderColorDark%;"
  "}"

  "WalletGui--TransactionDetailsDialog #m_transactionTab,"
  "WalletGui--TransactionDetailsDialog #m_transfersTab {"
    "background-color: #ffffff;"
    "border: none;"
  "}"

  "WalletGui--TransactionDetailsDialog #m_transfersView::item {"
    "padding-right: 0px;"
  "}";
}

TransactionDetailsDialog::TransactionDetailsDialog(ICryptoNoteAdapter* _cryptoNoteAdapter, QAbstractItemModel* _transactionsModel,
  const QModelIndex& _index, QWidget* _parent) : QDialog(_parent, static_cast<Qt::WindowFlags>(Qt::WindowCloseButtonHint)), m_ui(new Ui::TransactionDetailsDialog),
  m_cryptoNoteAdapter(_cryptoNoteAdapter), m_index() {
  m_ui->setupUi(this);
  m_index = _transactionsModel->index(_index.data(TransactionsModel::ROLE_ROW).toInt(), 0);
  QDataWidgetMapper* mapper = new QDataWidgetMapper(this);

  mapper->setModel(_transactionsModel);
  mapper->addMapping(m_ui->m_hashLabel, TransactionsModel::COLUMN_HASH, "text");
  mapper->addMapping(m_ui->m_statusLabel, TransactionsModel::COLUMN_STATE, "text");
  mapper->addMapping(m_ui->m_timeLabel, TransactionsModel::COLUMN_TIME, "text");
  mapper->addMapping(m_ui->m_heightLabel, TransactionsModel::COLUMN_HEIGHT, "text");
  mapper->addMapping(m_ui->m_toLabel, TransactionsModel::COLUMN_TRANSFERS, "text");
  mapper->addMapping(m_ui->m_amountLabel, TransactionsModel::COLUMN_AMOUNT, "text");
  mapper->addMapping(m_ui->m_feeLabel, TransactionsModel::COLUMN_FEE, "text");
  mapper->addMapping(m_ui->m_paymentIdLabel, TransactionsModel::COLUMN_PAYMENT_ID, "text");
  mapper->setCurrentModelIndex(m_index);
  TransfersModel* transfersModel = new TransfersModel(m_cryptoNoteAdapter, m_index, this);
  m_ui->m_transfersView->setModel(transfersModel);
  m_ui->m_transfersView->setHeader(new TransfersHeaderView(this));
  m_ui->m_transfersView->setItemDelegateForColumn(TransfersModel::COLUMN_ADDRESS, new CopyColumnDelegate(this));
  //m_ui->m_transfersView->setItemDelegateForColumn(TransfersModel::COLUMN_PROOF, new CopyColumnDelegate(this));
  m_ui->m_transfersView->setItemDelegateForColumn(TransfersModel::COLUMN_AMOUNT, new TransactionsAmountDelegate(false, this));
  m_ui->m_transfersView->header()->setSectionResizeMode(TransfersModel::COLUMN_ADDRESS, QHeaderView::Stretch);
  m_ui->m_transfersView->header()->setSectionResizeMode(TransfersModel::COLUMN_AMOUNT, QHeaderView::ResizeToContents);
  m_ui->m_transfersView->header()->setSectionResizeMode(TransfersModel::COLUMN_PROOF, QHeaderView::Fixed);
  m_ui->m_transfersView->header()->resizeSection(TransfersModel::COLUMN_PROOF, 80);
  m_ui->m_transfersView->setCopyableColumnSet(QSet<int>() << TransfersModel::COLUMN_ADDRESS << TransfersModel::COLUMN_PROOF);
  QString viewStyleSheet = m_ui->m_transfersView->styleSheet();
  for (int i = 0; i < transfersModel->rowCount(); ++i) {
    QModelIndex index = transfersModel->index(i, TransfersModel::COLUMN_ADDRESS);
    m_ui->m_transfersView->openPersistentEditor(index);
  }

  Crypto::SecretKey tx_key = m_cryptoNoteAdapter->getNodeAdapter()->getWalletAdapter()->getTransactionSecretKey(static_cast<size_t>(m_index.row()));
  QString transactionKey;
  if (tx_key != CryptoNote::NULL_SECRET_KEY) {
    transactionKey = QString::fromStdString(Common::podToHex(tx_key));
  } else {
    transactionKey = QString("(n/a)");
  }
  m_ui->m_txKeyLabel->setText(transactionKey);

  setStyleSheet(Settings::instance().getCurrentStyle().makeStyleSheet(TRANSACTION_DETAILS_DIALOG_STYLE_SHEET_TEMPLATE));
  m_ui->m_hashLabel->installEventFilter(this);
  connect(m_ui->m_transfersView, &WalletTreeView::copyableItemClickedSignal, this, &TransactionDetailsDialog::copyableItemClicked);
}

TransactionDetailsDialog::~TransactionDetailsDialog() {
}

bool TransactionDetailsDialog::eventFilter(QObject* _object, QEvent* _event) {
  if (_object == m_ui->m_hashLabel && _event->type() == QEvent::MouseButtonRelease) {
    QApplication::clipboard()->setText(m_ui->m_hashLabel->text());
     m_ui->m_copyLabel->start();
  }

  return QDialog::eventFilter(_object, _event);
}

void TransactionDetailsDialog::showEvent(QShowEvent* _event) {
  QDialog::showEvent(_event);
  adjustSize();
  setFixedWidth(width());
}

void TransactionDetailsDialog::copyableItemClicked(const QModelIndex& _index) {
  if (_index.column() == TransfersModel::COLUMN_PROOF) {
    QApplication::clipboard()->setText(getTxProof(_index));
  } else {
    QApplication::clipboard()->setText(_index.data().toString());
  }
}

QString TransactionDetailsDialog::getTxProof(const QModelIndex& _index) const {
  CryptoNote::WalletTransfer transfer = m_index.data(TransactionsModel::ROLE_TRANSFERS).
          value<QList<CryptoNote::WalletTransfer>>()[_index.row()];
  QString proof = "";
  Crypto::Hash txHash;
  Common::podFromHex(m_index.data(TransactionsModel::ROLE_HASH).toByteArray().toHex().toStdString(), txHash);
  Crypto::SecretKey txKey = m_cryptoNoteAdapter->getNodeAdapter()->getWalletAdapter()->getTransactionSecretKey(static_cast<size_t>(m_index.row()));
  QString address_str = QString::fromStdString(transfer.address);
  CryptoNote::AccountPublicAddress addr;
  if (m_cryptoNoteAdapter->parseAccountAddressString(address_str, addr) && transfer.amount > 0 && txKey != CryptoNote::NULL_SECRET_KEY) {
    proof = m_cryptoNoteAdapter->getTxProof(txHash, addr, txKey);
  }
  return proof;
}

}
