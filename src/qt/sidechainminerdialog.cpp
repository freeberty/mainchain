// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/sidechainminerdialog.h>
#include <qt/forms/ui_sidechainminerdialog.h>

#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>

#include <qt/drivenetunits.h>
#include <qt/guiconstants.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/sidechainactivationtablemodel.h>
#include <qt/sidechainescrowtablemodel.h>
#include <qt/wtprimevotetablemodel.h>
#include <qt/walletmodel.h>

#include <core_io.h>
#include <key.h>
#include <wallet/wallet.h>
#include <random.h>
#include <sidechain.h>
#include <sidechaindb.h>
#include <util.h>
#include <validation.h>

static const unsigned int INDEX_ESCROW = 0;
static const unsigned int INDEX_VOTE_SIDECHAIN = 1;
static const unsigned int INDEX_PROPOSE_SIDECHAIN = 2;
static const unsigned int INDEX_VOTE_WTPRIME = 3;
static const unsigned int INDEX_BMM_SETTINGS = 4;

enum DefaultWTPrimeVote {
    WTPRIME_UPVOTE = 0,
    WTPRIME_ABSTAIN = 1,
    WTPRIME_DOWNVOTE = 2,
};

SidechainMinerDialog::SidechainMinerDialog(const PlatformStyle *_platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SidechainMinerDialog),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);

    activationModel = new SidechainActivationTableModel(this);
    wtPrimeVoteModel = new WTPrimeVoteTableModel(this);

    ui->tableViewActivation->setModel(activationModel);
    ui->tableViewWTPrimeVote->setModel(wtPrimeVoteModel);

    // Set resize mode for WT^ vote table
    ui->tableViewWTPrimeVote->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Don't stretch last cell of horizontal header
    ui->tableViewActivation->horizontalHeader()->setStretchLastSection(false);
    ui->tableViewWTPrimeVote->horizontalHeader()->setStretchLastSection(false);

    // Hide vertical header
    ui->tableViewActivation->verticalHeader()->setVisible(false);
    ui->tableViewWTPrimeVote->verticalHeader()->setVisible(false);

    // Left align the horizontal header text
    ui->tableViewActivation->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tableViewWTPrimeVote->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    // Set horizontal scroll speed to per 3 pixels (very smooth, default is awful)
    ui->tableViewActivation->horizontalHeader()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableViewActivation->horizontalHeader()->horizontalScrollBar()->setSingleStep(3); // 3 Pixels
    ui->tableViewWTPrimeVote->horizontalHeader()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableViewWTPrimeVote->horizontalHeader()->horizontalScrollBar()->setSingleStep(3); // 3 Pixels

    // Select entire row
    ui->tableViewActivation->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewWTPrimeVote->setSelectionBehavior(QAbstractItemView::SelectRows);

    // If the user has WT^ vote parameters set, update the default vote combobox
    std::string strDefault = gArgs.GetArg("-defaultwtprimevote", "");
    if (strDefault == "upvote") {
        ui->comboBoxDefaultWTPrimeVote->setCurrentIndex(WTPRIME_UPVOTE);
    }
    else
    if (strDefault == "downvote") {
        ui->comboBoxDefaultWTPrimeVote->setCurrentIndex(WTPRIME_DOWNVOTE);
    }

    // Start the poll timer to update the page
    pollTimer = new QTimer(this);
    connect(pollTimer, SIGNAL(timeout()), this, SLOT(Update()));
    pollTimer->start(1000); // 1 second

    // Update the page
    Update();

    SetupTables();

    // Setup platform style single color icons

    // Buttons
    ui->pushButtonActivate->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_confirmed"));
    ui->pushButtonReject->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_conflicted"));
    ui->toolButtonACKSidechains->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_0"));
    ui->pushButtonUpvoteWTPrime->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_confirmed"));
    ui->pushButtonDownvoteWTPrime->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_conflicted"));
    ui->pushButtonAbstainWTPrime->setIcon(platformStyle->SingleColorIcon(":/icons/replay_not_replayed"));

    ui->toolButtonKeyHash->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_0"));
    ui->toolButtonSoftwareHashes->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_0"));
    ui->toolButtonIDHash1->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_0"));
    ui->toolButtonIDHash2->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_0"));
    ui->toolButtonVoteWTPrime->setIcon(platformStyle->SingleColorIcon(":/icons/transaction_0"));

    ui->comboBoxDefaultWTPrimeVote->setItemIcon(0, platformStyle->SingleColorIcon(":/icons/transaction_confirmed"));
    ui->comboBoxDefaultWTPrimeVote->setItemIcon(1, platformStyle->SingleColorIcon(":/icons/replay_not_replayed"));
    ui->comboBoxDefaultWTPrimeVote->setItemIcon(2, platformStyle->SingleColorIcon(":/icons/transaction_conflicted"));
}

SidechainMinerDialog::~SidechainMinerDialog()
{
    delete ui;
}

void SidechainMinerDialog::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if (model && model->getOptionsModel())
    {
        connect(model, SIGNAL(balanceChanged(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)), this,
                SLOT(setBalance(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)));
    }
}

void SidechainMinerDialog::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance,
                               const CAmount& immatureBalance, const CAmount& watchOnlyBalance,
                               const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    const CAmount& pending = immatureBalance + unconfirmedBalance;
    //ui->available->setText(BitcoinUnits::formatWithUnit(unit, balance, false, BitcoinUnits::separatorAlways));
    //ui->pending->setText(BitcoinUnits::formatWithUnit(unit, pending, false, BitcoinUnits::separatorAlways));
}

void SidechainMinerDialog::on_pushButtonEscrowStatus_clicked()
{
    ui->stackedWidget->setCurrentIndex(INDEX_ESCROW);
}

void SidechainMinerDialog::on_pushButtonVoteSidechain_clicked()
{
    ui->stackedWidget->setCurrentIndex(INDEX_VOTE_SIDECHAIN);
}

void SidechainMinerDialog::on_pushButtonProposeSidechain_clicked()
{
    ui->stackedWidget->setCurrentIndex(INDEX_PROPOSE_SIDECHAIN);
}

void SidechainMinerDialog::on_pushButtonVoteWTPrime_clicked()
{
    ui->stackedWidget->setCurrentIndex(INDEX_VOTE_WTPRIME);
}

void SidechainMinerDialog::on_pushButtonCreateSidechainProposal_clicked()
{
    std::string strTitle = ui->lineEditTitle->text().toStdString();
    std::string strDescription = ui->plainTextEditDescription->toPlainText().toStdString();
    std::string strHash = ui->lineEditHash->text().toStdString();
    std::string strHashID1 = ui->lineEditIDHash1->text().toStdString();
    std::string strHashID2 = ui->lineEditIDHash2->text().toStdString();
    int nVersion = ui->spinBoxVersion->value();

    if (strTitle.empty()) {
        QMessageBox::critical(this, tr("DriveNet - error"),
            tr("Sidechain must have a title!"),
            QMessageBox::Ok);
        return;
    }

    // TODO maybe we should allow sidechains with no description? Anyways this
    // isn't a consensus rule right now
    if (strDescription.empty()) {
        QMessageBox::critical(this, tr("DriveNet - error"),
            tr("Sidechain must have a description!"),
            QMessageBox::Ok);
        return;
    }

    if (nVersion > SIDECHAIN_VERSION_MAX) {
        QMessageBox::critical(this, tr("DriveNet - error"),
            tr("This sidechain has an invalid version number (too high)!"),
            QMessageBox::Ok);
        return;
    }

    uint256 uHash = uint256S(strHash);
    if (uHash.IsNull()) {
        QMessageBox::critical(this, tr("DriveNet - error"),
            tr("Invalid sidechain key hash!"),
            QMessageBox::Ok);
        return;
    }

    CKey key;
    key.Set(uHash.begin(), uHash.end(), false);

    CBitcoinSecret vchSecret(key);

    if (!key.IsValid()) {
        // Nobody should see this, but we don't want to fail silently
        QMessageBox::critical(this, tr("DriveNet - error"),
            tr("Private key outside allowed range!"),
            QMessageBox::Ok);
        return;
    }

    CPubKey pubkey = key.GetPubKey();
    assert(key.VerifyPubKey(pubkey));
    CKeyID vchAddress = pubkey.GetID();

    if (!strHashID1.empty() && strHashID1.size() != 64) {
        QMessageBox::critical(this, tr("DriveNet - error"),
            tr("HashID1 (release tarball hash) invalid size!"),
            QMessageBox::Ok);
        return;
    }
    if (!strHashID2.empty() && strHashID2.size() != 40) {
        QMessageBox::critical(this, tr("DriveNet - error"),
            tr("HashID2 (build commit hash) invalid size!"),
            QMessageBox::Ok);
        return;
    }

    // Generate script hex
    CScript sidechainScript = CScript() << OP_DUP << OP_HASH160 << ToByteVector(vchAddress) << OP_EQUALVERIFY << OP_CHECKSIG;

    SidechainProposal proposal;
    proposal.title = strTitle;
    proposal.description = strDescription;
    proposal.sidechainPriv = vchSecret.ToString();
    proposal.sidechainKeyID = HexStr(vchAddress);
    proposal.sidechainHex = HexStr(sidechainScript);
    if (!strHashID1.empty())
        proposal.hashID1 = uint256S(strHashID1);
    if (!strHashID1.empty())
        proposal.hashID2 = uint160S(strHashID2);
    proposal.nVersion = nVersion;

    // Cache proposal so that it can be added to the next block we mine
    scdb.CacheSidechainProposals(std::vector<SidechainProposal>{proposal});

    QString message = QString("Sidechain proposal created!\n\n");
    message += QString("Version:\n%1\n\n").arg(nVersion);
    message += QString("Title:\n%1\n\n").arg(QString::fromStdString(strTitle));
    message += QString("Description:\n%1\n\n").arg(QString::fromStdString(strDescription));
    message += QString("Private key:\n%1\n\n").arg(QString::fromStdString(proposal.sidechainPriv));
    message += QString("KeyID:\n%1\n\n").arg(QString::fromStdString(proposal.sidechainKeyID));
    message += QString("Deposit script:\n%1\n\n").arg(QString::fromStdString(proposal.sidechainHex));

    std::vector<unsigned char> vch(ParseHex(proposal.sidechainHex));
    CScript scriptPubKey = CScript(vch.begin(), vch.end());
    message += QString("Deposit script asm:\n%1\n\n").arg(QString::fromStdString(ScriptToAsmStr(scriptPubKey)));

    if (!strHashID1.empty())
        message += QString("Hash ID 1:\n%1\n\n").arg(QString::fromStdString(strHashID1));
    if (!strHashID2.empty())
        message += QString("Hash ID 2:\n%1\n\n").arg(QString::fromStdString(strHashID2));

    message += "Note: you can use the RPC command 'listsidechainproposals' to" \
    " view your pending sidechain proposals or 'listactivesidechains' to view" \
    " active sidechains.\n";

    // Show result message popup
    QMessageBox::information(this, tr("DriveNet - sidechain proposal created!"),
        message,
        QMessageBox::Ok);

    ui->lineEditTitle->clear();
    ui->plainTextEditDescription->clear();
    ui->lineEditHash->clear();
    ui->lineEditIDHash1->clear();
    ui->lineEditIDHash2->clear();
    ui->spinBoxVersion->setValue(0);
}

void SidechainMinerDialog::on_pushButtonActivate_clicked()
{
    QModelIndexList selected = ui->tableViewActivation->selectionModel()->selectedIndexes();

    for (int i = 0; i < selected.size(); i++) {
        uint256 hash;
        if (activationModel->GetHashAtRow(selected[i].row(), hash))
            scdb.CacheSidechainHashToActivate(hash);
    }
}

void SidechainMinerDialog::on_pushButtonReject_clicked()
{
    QModelIndexList selected = ui->tableViewActivation->selectionModel()->selectedIndexes();

    for (int i = 0; i < selected.size(); i++) {
        uint256 hash;
        if (activationModel->GetHashAtRow(selected[i].row(), hash))
            scdb.RemoveSidechainHashToActivate(hash);
    }
}

// TODO
// refactor all of the voting pushButton slots to use one function for
// setting WT^ vote type.

void SidechainMinerDialog::on_pushButtonUpvoteWTPrime_clicked()
{
    // Set WT^ vote type

    QModelIndexList selected = ui->tableViewWTPrimeVote->selectionModel()->selectedIndexes();

    for (int i = 0; i < selected.size(); i++) {
        uint256 hash;
        unsigned int nSidechain;
        if (wtPrimeVoteModel->GetWTPrimeInfoAtRow(selected[i].row(), hash, nSidechain)) {
            SidechainCustomVote vote;
            vote.nSidechain = nSidechain;
            vote.hashWTPrime = hash;
            vote.vote = SCDB_UPVOTE;

            scdb.CacheCustomVotes(std::vector<SidechainCustomVote>{ vote });
        }
    }
}

void SidechainMinerDialog::on_pushButtonDownvoteWTPrime_clicked()
{
    // Set WT^ vote type

    QModelIndexList selected = ui->tableViewWTPrimeVote->selectionModel()->selectedIndexes();

    for (int i = 0; i < selected.size(); i++) {
        uint256 hash;
        unsigned int nSidechain;
        if (wtPrimeVoteModel->GetWTPrimeInfoAtRow(selected[i].row(), hash, nSidechain)) {
            SidechainCustomVote vote;
            vote.nSidechain = nSidechain;
            vote.hashWTPrime = hash;
            vote.vote = SCDB_DOWNVOTE;

            scdb.CacheCustomVotes(std::vector<SidechainCustomVote>{ vote });
        }
    }
}

void SidechainMinerDialog::on_pushButtonAbstainWTPrime_clicked()
{
    // Set WT^ vote type

    QModelIndexList selected = ui->tableViewWTPrimeVote->selectionModel()->selectedIndexes();

    for (int i = 0; i < selected.size(); i++) {
        uint256 hash;
        unsigned int nSidechain;
        if (wtPrimeVoteModel->GetWTPrimeInfoAtRow(selected[i].row(), hash, nSidechain)) {
            SidechainCustomVote vote;
            vote.nSidechain = nSidechain;
            vote.hashWTPrime = hash;
            vote.vote = SCDB_ABSTAIN;

            scdb.CacheCustomVotes(std::vector<SidechainCustomVote>{ vote });
        }
    }
}

void SidechainMinerDialog::on_pushButtonClose_clicked()
{
    this->close();
}

void SidechainMinerDialog::on_toolButtonACKSidechains_clicked()
{
    // TODO move text into static const
    QMessageBox::information(this, tr("DriveNet - information"),
        tr("Sidechain activation signalling:\n\n"
           "Use this page to ACK (acknowledgement) or NACK "
           "(negative-acknowledgement) sidechains.\n\n"
           "Set ACK to activate a proposed sidechain, and NACK to reject a "
           "proposed sidechain.\n\n"
           "Once set, the chosen signal will be included in blocks mined by "
           "this node."),
        QMessageBox::Ok);
}

void SidechainMinerDialog::on_toolButtonACKWTPrime_clicked()
{
    // TODO move text into static const
    QMessageBox::information(this, tr("DriveNet - information"),
        tr("WT^ voting:\n\n"
           "Use this page to ACK (acknowledgement) or NACK "
           "(negative-acknowledgement) or abstain (ignore) WT^(s).\n\n"
           "Set ACK to commit work to the WT^, and NACK to downvote it.\n\n"
           "Once set, the chosen signal will be included in blocks mined by "
           "this node."),
        QMessageBox::Ok);
}

void SidechainMinerDialog::on_toolButtonKeyHash_clicked()
{
    // TODO move text into static const
    QMessageBox::information(this, tr("DriveNet - information"),
        tr("Sidechain address bytes:\n\n"
           "Deposits to this sidechain must be sent to a specific address "
           "(really, a specific script). It must be different from the "
           "addresses in use by active sidechains.\n\n"
           "Each sidechain must use a unique address or the sidechain software "
           "will be confused.\n\n"
           "The address will be based on 256 bits (encoded as 32 bytes of hex) "
           "- you get to choose what these bits are.\n\n"
           "Add the address bytes to the src/sidechain.h file of the sidechain."
           "\n\n"
           "Example:\n"
           "static const std::string SIDECHAIN_ADDRESS_BYTES = \"6e1f86cb9785d4484750970c7f4cd42a142d3c50974a0a3128f562934774b191\";"),
        QMessageBox::Ok);
}

void SidechainMinerDialog::on_toolButtonIDHash1_clicked()
{
    // TODO display message based on current selected version
    // TODO move text into static const
    QMessageBox::information(this, tr("DriveNet - information"),
        tr("Release tarball hash:\n\n"
           "hash of the original gitian software build of this sidechain.\n\n"
           "Use the sha256sum utility to generate this hash, or copy the hash "
           "when it is printed to the console after gitian builds complete.\n\n"
           "Example:\n"
           "sha256sum DriveNet-12-0.21.00-x86_64-linux-gnu.tar.gz\n\n"
           "Result:\n"
           "fd9637e427f1e967cc658bfe1a836d537346ce3a6dd0746878129bb5bc646680  DriveNet-12-0.21.00-x86_64-linux-gnu.tar.gz\n\n"
           "Paste the resulting hash into this field."),
        QMessageBox::Ok);
}

void SidechainMinerDialog::on_toolButtonIDHash2_clicked()
{
    // TODO display message based on current selected version
    // TODO move text into static const
    QMessageBox::information(this, tr("DriveNet - information"),
        tr("Build commit hash (160 bits):\n\n"
           "If the software was developed using git, the build commit hash "
           "should match the commit hash of the first sidechain release.\n\n"
           "To verify it later, you can look up this commit in the repository "
           "history."),
        QMessageBox::Ok);
}

void SidechainMinerDialog::on_toolButtonSoftwareHashes_clicked()
{
    // TODO display message based on current selected version
    // TODO move text into static const
    QMessageBox::information(this, tr("DriveNet - information"),
        tr("These help users find the sidechain node software. "
           "Only this software can filter out invalid WT^s. \n\n"
           "These fields are optional but highly recommended."),
        QMessageBox::Ok);
}

void SidechainMinerDialog::on_toolButtonVoteWTPrime_clicked()
{
    // TODO move text into static const
    QMessageBox::information(this, tr("DriveNet - information"),
        tr("Sidechain WT^ vote signalling:\n\n"
           "Use this page to set votes for WT^(s).\n\n"
           "Set Upvote to increase the work score of WT^(s) in blocks "
           "that you mine. Downvote to decrease the work score, and Abstain "
           "to ignore a WT^ and not change its workscore.\n\n"
           "You may also use the RPC command 'setwtprimevote' to set votes "
           "or 'clearwtprimevotes' to reset and erase any votes you have set."
           ),
        QMessageBox::Ok);
}

void SidechainMinerDialog::on_pushButtonRandomKeyHash_clicked()
{
    uint256 hash = GetRandHash();
    ui->lineEditHash->setText(QString::fromStdString(hash.ToString()));
}

void SidechainMinerDialog::on_comboBoxDefaultWTPrimeVote_currentIndexChanged(const int i)
{
    if (i == WTPRIME_UPVOTE) {
        gArgs.ForceSetArg("-defaultwtprimevote", "upvote");
    }
    else
    if (i == WTPRIME_ABSTAIN) {
        gArgs.ForceSetArg("-defaultwtprimevote", "abstain");
    }
    else
    if (i == WTPRIME_DOWNVOTE) {
        gArgs.ForceSetArg("-defaultwtprimevote", "downvote");
    }
}

void SidechainMinerDialog::Update()
{
    // Disable the default vote combo box if custom votes are set, enable it
    // if they are not.
    std::vector<SidechainCustomVote> vCustomVote = scdb.GetCustomVoteCache();
    bool fCustomVote = vCustomVote.size();
    ui->comboBoxDefaultWTPrimeVote->setEnabled(!fCustomVote);

    // Add a tip to the default vote label on how to reset them
    ui->labelClearVotes->setHidden(!fCustomVote);
}

void SidechainMinerDialog::SetupTables()
{
    if (escrowModel)
        delete escrowModel;

    // Initialize table models
    escrowModel = new SidechainEscrowTableModel(this);

    // Add models to table views
    ui->tableViewEscrow->setModel(escrowModel);

    // Resize cells (in a backwards compatible way)
#if QT_VERSION < 0x050000
    ui->tableViewEscrow->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
    ui->tableViewEscrow->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif

    // Don't stretch last cell of horizontal header
    ui->tableViewEscrow->horizontalHeader()->setStretchLastSection(false);

    // Hide vertical header
    ui->tableViewEscrow->verticalHeader()->setVisible(false);

    // Left align the horizontal header text
    ui->tableViewEscrow->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    // Set horizontal scroll speed to per 3 pixels (very smooth, default is awful)
    ui->tableViewEscrow->horizontalHeader()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableViewEscrow->horizontalHeader()->horizontalScrollBar()->setSingleStep(3); // 3 Pixels

    // Disable word wrap
    ui->tableViewEscrow->setWordWrap(false);
}
