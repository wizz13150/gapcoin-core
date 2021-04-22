// Copyright (c) 2020 The Gapcoin Core developers
// Copyright (c) 2016-2018 Duality Blockchain Solutions Developers
// Copyright (c) 2014-2018 The Dash Core Developers
// Copyright (c) 2009-2018 The Bitcoin Developers
// Copyright (c) 2009-2018 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/miningpage.h>
#include <qt/forms/ui_miningpage.h>
#include <qt/clientmodel.h>
#include <qt/guiutil.h>
#include <qt/walletmodel.h>

#include <bignum.h>
#include <miner.h>
#include <net.h>
#include <pow.h>
#include <rpc/server.h>
#include <univalue.h>
#include <util.h>
#include <utiltime.h>
#include <validation.h>

#include <boost/thread.hpp>
#include <stdio.h>

#include <QtDebug>

extern UniValue GetNetworkHashPS(int lookup, int height);

// Mapping of shifts to primedigits
int shiftmap[500] = {81,82,82,82,83,83,83,84,84,84,84,85,85,85,86,
    86,86,87,87,87,87,88,88,88,89,89,89,90,90,90,91,91,91,91,92,
    92,92,93,93,93,94,94,94,94,95,95,95,96,96,96,97,97,97,97,98,
    98,98,99,99,99,100,100,100,100,101,101,101,102,102,102,103,103,
    103,103,104,104,104,105,105,105,106,106,106,106,107,107,107,108,
    108,108,109,109,109,109,110,110,110,111,111,111,112,112,112,112,
    113,113,113,114,114,114,115,115,115,115,116,116,116,117,117,117,
    118,118,118,119,119,119,119,120,120,120,121,121,121,122,122,122,
    122,123,123,123,124,124,124,125,125,125,125,126,126,126,127,127,
    127,128,128,128,128,129,129,129,130,130,130,131,131,131,131,132,
    132,132,133,133,133,134,134,134,134,135,135,135,136,136,136,137,
    137,137,137,138,138,138,139,139,139,140,140,140,140,141,141,141,
    142,142,142,143,143,143,143,144,144,144,145,145,145,146,146,146,
    146,147,147,147,148,148,148,149,149,149,150,150,150,150,151,151,
    151,152,152,152,153,153,153,153,154,154,154,155,155,155,156,156,
    156,156,157,157,157,158,158,158,159,159,159,159,160,160,160,161,
    161,161,162,162,162,162,163,163,163,164,164,164,165,165,165,165,
    166,166,166,167,167,167,168,168,168,168,169,169,169,170,170,170,
    171,171,171,171,172,172,172,173,173,173,174,174,174,174,175,175,
    175,176,176,176,177,177,177,178,178,178,178,179,179,179,180,180,
    180,181,181,181,181,182,182,182,183,183,183,184,184,184,184,185,
    185,185,186,186,186,187,187,187,187,188,188,188,189,189,189,190,
    190,190,190,191,191,191,192,192,192,193,193,193,193,194,194,194,
    195,195,195,196,196,196,196,197,197,197,198,198,198,199,199,199,
    199,200,200,200,201,201,201,202,202,202,202,203,203,203,204,204,
    204,205,205,205,206,206,206,206,207,207,207,208,208,208,209,209,
    209,209,210,210,210,211,211,211,212,212,212,212,213,213,213,214,
    214,214,215,215,215,215,216,216,216,217,217,217,218,218,218,218,
    219,219,219,220,220,220,221,221,221,221,222,222,222,223,223,223,
    224,224,224,224,225,225,225,226,226,226,227,227,227,227,228,228,
    228,229,229,229,230,230,230,230,231,231,231,232};

MiningPage::MiningPage(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MiningPage),
    hasMiningprivkey(false)
{
    ui->setupUi(this);
    this->powUtils = new PoWUtils;
    int nMaxUseThreads = GUIUtil::MaxThreads();

    /*
    std::string PrivAddress = gArgs.GetArg("-miningprivkey", "");
    if (!PrivAddress.empty())
    {
        CBitcoinSecret Secret;
        Secret.SetString(PrivAddress);
        if (Secret.IsValid())
        {
            CBitcoinAddress Address;
            Address.Set(Secret.GetKey().GetPubKey().GetID());
            ui->labelAddress->setText(QString("All mined coins will go to %1").arg(Address.ToString().c_str()));
            hasMiningprivkey = true;
        }
    }
    */

    ui->sliderCores->setMinimum(0);
    ui->sliderCores->setMaximum(nMaxUseThreads);
    ui->sliderCores->setValue(nMaxUseThreads);
    ui->labelNCores->setText(QString("%1").arg(nMaxUseThreads));
    ui->sliderGraphSampleTime->setMaximum(0);
    ui->sliderGraphSampleTime->setMaximum(6);

    ui->sliderCores->setToolTip(tr("Use the slider to select the amount of CPU threads to use."));
    ui->labelNetHashRate->setToolTip(tr("This shows the overall hashrate of the Gapcoin network."));
    ui->labelMinerHashRate->setToolTip(tr("This shows the hashrate of your CPU whilst mining."));
    ui->labelNextBlock->setToolTip(tr("This shows the average time between the blocks you have mined."));

    isMining = gArgs.GetBoolArg("-gen", false)? 1 : 0;

    QIntValidator *headershiftValuevalidator = new QIntValidator(14, 512, this);
    ui->headershiftValue->setValidator(headershiftValuevalidator);
    QIntValidator *sievesizeValuevalidator = new QIntValidator(1000, INT_MAX, this);
    ui->sievesizeValue->setValidator(sievesizeValuevalidator);
    QIntValidator *sieveprimesValuevalidator = new QIntValidator(1000, INT_MAX, this);
    ui->sieveprimesValue->setValidator(sieveprimesValuevalidator);

    connect(ui->sliderCores, SIGNAL(valueChanged(int)), this, SLOT(changeNumberOfCores(int)));
    connect(ui->sliderGraphSampleTime, SIGNAL(valueChanged(int)), this, SLOT(changeSampleTime(int)));
    connect(ui->pushSwitchMining, SIGNAL(clicked()), this, SLOT(switchMining()));
    connect(ui->pushButtonClearData, SIGNAL(clicked()), this, SLOT(clearHashRateData()));
    connect(ui->checkBoxShowGraph, SIGNAL(stateChanged(int)), this, SLOT(showHashRate(int)));
    connect(ui->headershiftValue, SIGNAL(editingFinished()), this, SLOT(shiftChanged()));

    ui->minerHashRateWidget->graphType = HashRateGraphWidget::GraphType::MINER_HASHRATE;
    ui->minerHashRateWidget->UpdateSampleTime(HashRateGraphWidget::SampleTime::FIVE_MINUTES);
    
    showHashMeterControls(false);

    updateUI(isMining);
    startTimer(8000);
}

MiningPage::~MiningPage()
{
    delete ui;
}

void MiningPage::setModel(WalletModel *model)
{
    this->model = model;
}

void MiningPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
}

void MiningPage::updateUI(bool fGenerate)
{

    qint64 NetworkHashrate = GUIUtil::GetNetworkHashPS(120, -1);
    // int64_t hashrate = (int64_t)dHashesPerSec;
    qint64 Hashrate = GUIUtil::GetHashRate();

    ui->labelNetHashRate->setText(GUIUtil::FormatHashRate(NetworkHashrate));
    ui->labelMinerHashRate->setText(GUIUtil::FormatHashRate(Hashrate));
    
    QString NextBlockTime;
    if (Hashrate == 0)
        NextBlockTime = QChar(L'∞');
    else
    {
        double gaps_per_day = this->powUtils->gaps_per_day(dHashesPerSec, chainActive.Tip()->nDifficulty);
        NextBlockTime = QString::number(gaps_per_day);
    }
    ui->labelNextBlock->setText(NextBlockTime);

    if (GUIUtil::GetHashRate() == 0) {
        ui->pushSwitchMining->setToolTip(tr("Click 'Start mining' to begin mining."));
        ui->pushSwitchMining->setText(tr("Start mining."));
        ui->pushSwitchMining->setEnabled(true);
     }
     else {
        ui->pushSwitchMining->setToolTip(tr("Click 'Stop mining' to finish mining."));
        ui->pushSwitchMining->setText(tr("Stop mining."));
        ui->pushSwitchMining->setEnabled(true);
    }
    ui->pushSwitchMining->setEnabled(true);

    QString status = QString("Not Mining Gapcoin");
    if (fGenerate)
        status = QString("Mining with %1/%2 threads, shift: %3, sieve size: %4, number of primes in sieve: %5 - hashrate: %6 (%7 tests per sec.)")
                .arg((int)ui->sliderCores->value())
                .arg(GUIUtil::MaxThreads())
                .arg(nMiningShift).arg(nMiningSieveSize)
                .arg(nMiningPrimes)
                .arg(GUIUtil::FormatHashRate(Hashrate)).arg(dTestsPerSec);
    ui->miningStatistics->setText(status);
}

void MiningPage::restartMining(bool fGenerate, int nThreads)
{
    isMining = fGenerate;
    if (nThreads <= maxGenProc)
        nUseThreads = nThreads;

    // unlock wallet before mining

  #ifndef __linux__
    if (fGenerate && !hasMiningprivkey && !unlockContext.get())
    {
        this->unlockContext.reset(new WalletModel::UnlockContext(model->requestUnlock()));
        if (!unlockContext->isValid())
        {
            unlockContext.reset(nullptr);
            return;
        }
    }
  #endif

    nMiningShift = ui->headershiftValue->text().toInt();
    nMiningSieveSize = ui->sievesizeValue->text().toInt();
    nMiningPrimes = ui->sieveprimesValue->text().toInt();

    GenerateGapcoins(true, nThreads, Params());

    // lock wallet after mining
    if (!fGenerate && !hasMiningprivkey)
        unlockContext.reset(nullptr);

    updateUI(fGenerate);
}

void MiningPage::StartMiner()
{
    int nThreads = (int)ui->sliderCores->value();
    nMiningShift = ui->headershiftValue->text().toInt();
    nMiningSieveSize = ui->sievesizeValue->text().toInt();
    nMiningPrimes = ui->sieveprimesValue->text().toInt();
    if (nMiningShift < 64 && nMiningSieveSize > (((uint64_t) 1) << nMiningShift))
       nMiningSieveSize = (((uint64_t) 1) << nMiningShift);

    GenerateGapcoins(true, nThreads, Params());
    isMining = true;
    updateUI(isMining);
}

void MiningPage::StopMiner()
{
    isMining = false;
    int nThreads = (int)ui->sliderCores->value();
    GenerateGapcoins(false, nThreads, Params());
    updateUI(isMining);
}

void MiningPage::changeNumberOfCores(int i)
{
    // restartMining(isMining, i);

    ui->labelNCores->setText(QString("%1").arg(i));
    if (i == 0) {
        StopMiner();
    }
    else if (i > 0 && GUIUtil::GetHashRate() > 0) {  
        StartMiner();
    }
}

void MiningPage::switchMining()
{
    // restartMining(!isMining, ui->sliderCores->value());

    int64_t hashRate = GUIUtil::GetHashRate();
    int nThreads = (int)ui->sliderCores->value();
    
    if (hashRate > 0) {
        ui->pushSwitchMining->setText(tr("Stopping."));
        StopMiner();
    }
    else if (nThreads == 0 && hashRate == 0){
        ui->sliderCores->setValue(1);
        ui->pushSwitchMining->setText(tr("Starting."));
        StartMiner();
    }
    else {
        ui->pushSwitchMining->setText(tr("Starting."));
        StartMiner();
    }
}

void MiningPage::timerEvent(QTimerEvent *)
{
    updateUI(isMining);
}

void MiningPage::updateSievePrimes(int i)
{
    nMiningShift = i;
    if (nMiningShift < 64 && nMiningSieveSize > (((uint64_t) 1) << nMiningShift)) {
       nMiningSieveSize = (((uint64_t) 1) << nMiningShift);
        qDebug() << "New header shift: " << QString("%1").arg(i) << ", new sieve size value: " << QString("%1").arg(nMiningSieveSize);
        ui->sievesizeValue->setText(QString("%1").arg(nMiningSieveSize));
    }
}

void MiningPage::shiftChanged()
{
    bool ok;
    QString newshift = ui->headershiftValue->text();
    int shift = newshift.toInt(&ok, 10);
    int primedigits = shiftmap[shift-15];
    newshift = QString::number(primedigits);
    ui->primedigitValue->setText(QString("%1").arg(newshift));
    updateSievePrimes(shift);
}

void MiningPage::showHashRate(int i)
{
    if (i == 0) {
        ui->minerHashRateWidget->StopHashMeter();
        showHashMeterControls(false);
    }
    else {
        ui->minerHashRateWidget->StartHashMeter();
        showHashMeterControls(true);
    }
}

void MiningPage::showHashMeterControls(bool show)
{
    if (show == false) {
        ui->sliderGraphSampleTime->setVisible(false);
        ui->labelGraphSampleSize->setVisible(false);
        ui->pushButtonClearData->setVisible(false);
    }
    else {
        ui->sliderGraphSampleTime->setVisible(true);
        ui->labelGraphSampleSize->setVisible(true);
        ui->pushButtonClearData->setVisible(true);
    }
}

void MiningPage::changeSampleTime(int i)
{
    if (i == 0) {
        ui->minerHashRateWidget->UpdateSampleTime(HashRateGraphWidget::SampleTime::FIVE_MINUTES);
        ui->labelGraphSampleSize->setText(QString("5 minutes"));
    }
    else if (i == 1) {
        ui->minerHashRateWidget->UpdateSampleTime(HashRateGraphWidget::SampleTime::TEN_MINUTES);
        ui->labelGraphSampleSize->setText(QString("10 minutes"));
    }
    else if (i == 2) {
        ui->minerHashRateWidget->UpdateSampleTime(HashRateGraphWidget::SampleTime::THIRTY_MINUTES);
        ui->labelGraphSampleSize->setText(QString("30 minutes"));
    }
    else if (i == 3) {
        ui->minerHashRateWidget->UpdateSampleTime(HashRateGraphWidget::SampleTime::ONE_HOUR);
        ui->labelGraphSampleSize->setText(QString("1 hour"));
    }
    else if (i == 4) {
        ui->minerHashRateWidget->UpdateSampleTime(HashRateGraphWidget::SampleTime::EIGHT_HOURS);
        ui->labelGraphSampleSize->setText(QString("8 hours"));
    }
    else if (i == 5) {
        ui->minerHashRateWidget->UpdateSampleTime(HashRateGraphWidget::SampleTime::TWELVE_HOURS);
        ui->labelGraphSampleSize->setText(QString("12 hours"));
    }
    else if (i == 6) {
        ui->minerHashRateWidget->UpdateSampleTime(HashRateGraphWidget::SampleTime::ONE_DAY);
        ui->labelGraphSampleSize->setText(QString("1 day"));
    }
    else {
        ui->minerHashRateWidget->UpdateSampleTime(HashRateGraphWidget::SampleTime::ONE_DAY);
        ui->labelGraphSampleSize->setText(QString("1 day"));
    }
}

void MiningPage::clearHashRateData()
{
    ui->minerHashRateWidget->clear();
}
