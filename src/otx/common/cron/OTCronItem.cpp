// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "internal/otx/common/cron/OTCronItem.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "internal/api/Legacy.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Exclusive.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/consensus/Client.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/common/OTStorage.hpp"

// Base class for OTTrade and OTAgreement and OTPaymentPlan.
// OTCron contains lists of these for regular processing.

// static -- class factory.
//
// I just realized, I don't have to use this only for CronItems.
// If I wanted to, I could put ANY Open-Transactions class in here,
// if there was some need for it, and it would work just fine right here.
// Like if I wanted to have different Token types for different cash
// algorithms. All I have to do is change the return type.
//

namespace opentxs
{
OTCronItem::OTCronItem(const api::Session& api)
    : ot_super(api)
    , m_dequeClosingNumbers{}
    , m_pCancelerNymID(api_.Factory().NymID())
    , m_bCanceled(false)
    , m_bRemovalFlag(false)
    , m_pCron(nullptr)
    , serverNym_(nullptr)
    , notaryID_(api_.Factory().Identifier())
    , m_CREATION_DATE()
    , m_LAST_PROCESS_DATE()
    , m_PROCESS_INTERVAL(1)
{
    InitCronItem();
}

OTCronItem::OTCronItem(
    const api::Session& api,
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    : ot_super(api, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_dequeClosingNumbers{}
    , m_pCancelerNymID(api_.Factory().NymID())
    , m_bCanceled(false)
    , m_bRemovalFlag(false)
    , m_pCron(nullptr)
    , serverNym_(nullptr)
    , notaryID_(api_.Factory().Identifier())
    , m_CREATION_DATE()
    , m_LAST_PROCESS_DATE()
    , m_PROCESS_INTERVAL(1)
{
    InitCronItem();
}

OTCronItem::OTCronItem(
    const api::Session& api,
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const Identifier& ACCT_ID,
    const identifier::Nym& NYM_ID)
    : ot_super(api, NOTARY_ID, INSTRUMENT_DEFINITION_ID, ACCT_ID, NYM_ID)
    , m_dequeClosingNumbers{}
    , m_pCancelerNymID(api_.Factory().NymID())
    , m_bCanceled(false)
    , m_bRemovalFlag(false)
    , m_pCron(nullptr)
    , serverNym_(nullptr)
    , notaryID_(api_.Factory().Identifier())
    , m_CREATION_DATE()
    , m_LAST_PROCESS_DATE()
    , m_PROCESS_INTERVAL(1)

{
    InitCronItem();
}

auto OTCronItem::LoadCronReceipt(
    const api::Session& api,
    const TransactionNumber& lTransactionNum) -> std::unique_ptr<OTCronItem>
{
    auto filename = api::Legacy::GetFilenameCrn(lTransactionNum);

    const char* szFoldername = api.Internal().Legacy().Cron();
    if (!OTDB::Exists(api, api.DataFolder(), szFoldername, filename, "", "")) {
        LogError()(OT_PRETTY_STATIC(OTCronItem))("File does not exist: ")(
            szFoldername)(api::Legacy::PathSeparator())(filename)(".")
            .Flush();
        return nullptr;
    }

    auto strFileContents = String::Factory(OTDB::QueryPlainString(
        api, api.DataFolder(), szFoldername, filename, "", ""));  // <===
                                                                  // LOADING
                                                                  // FROM
                                                                  // DATA
                                                                  // STORE.

    if (strFileContents->GetLength() < 2) {
        LogError()(OT_PRETTY_STATIC(OTCronItem))("Error reading file: ")(
            szFoldername)(api::Legacy::PathSeparator())(filename)(".")
            .Flush();
        return nullptr;
    } else {
        // NOTE: NewCronItem can handle the normal cron item contracts, as well
        // as the OT ARMORED version
        // (It will decode the armor before instantiating the contract.)
        // Therefore there's no need HERE in
        // THIS function to do any decoding...
        //
        return api.Factory().InternalSession().CronItem(strFileContents);
    }
}

// static
auto OTCronItem::LoadActiveCronReceipt(
    const api::Session& api,
    const TransactionNumber& lTransactionNum,
    const identifier::Notary& notaryID)
    -> std::unique_ptr<OTCronItem>  // Client-side only.
{
    auto strNotaryID = String::Factory(notaryID);
    auto filename = api::Legacy::GetFilenameCrn(lTransactionNum);

    const char* szFoldername = api.Internal().Legacy().Cron();

    if (!OTDB::Exists(
            api,
            api.DataFolder(),
            szFoldername,
            strNotaryID->Get(),
            filename,
            "")) {
        LogError()(OT_PRETTY_STATIC(OTCronItem))("File does not exist: ")(
            szFoldername)(api::Legacy::PathSeparator())(
            strNotaryID)(api::Legacy::PathSeparator())(filename)(".")
            .Flush();
        return nullptr;
    }

    auto strFileContents = String::Factory(OTDB::QueryPlainString(
        api,
        api.DataFolder(),
        szFoldername,
        strNotaryID->Get(),
        filename,
        ""));  // <=== LOADING FROM
               // DATA STORE.

    if (strFileContents->GetLength() < 2) {
        LogError()(OT_PRETTY_STATIC(OTCronItem))("Error reading file: ")(
            szFoldername)(api::Legacy::PathSeparator())(
            strNotaryID)(api::Legacy::PathSeparator())(filename)(".")
            .Flush();
        return nullptr;
    } else {
        // NOTE: NewCronItem can handle the normal cron item contracts, as well
        // as the OT ARMORED version
        // (It will decode the armor before instantiating the contract.)
        // Therefore there's no need HERE in
        // THIS function to do any decoding...
        //
        return api.Factory().InternalSession().CronItem(strFileContents);
    }
}

// static
// Client-side only.
auto OTCronItem::GetActiveCronTransNums(
    const api::Session& api,
    NumList& output,
    const UnallocatedCString& dataFolder,
    const identifier::Nym& nymID,
    const identifier::Notary& notaryID) -> bool
{
    const char* szFoldername = api.Internal().Legacy().Cron();

    output.Release();

    // We need to load up the local list of active (recurring) transactions.
    //
    auto strNotaryID = String::Factory(notaryID);
    auto filename = api::Legacy::GetFilenameLst(nymID.str());

    if (OTDB::Exists(
            api, dataFolder, szFoldername, strNotaryID->Get(), filename, "")) {
        // Load up existing list, if it exists.
        //
        auto strNumlist = String::Factory(OTDB::QueryPlainString(
            api, dataFolder, szFoldername, strNotaryID->Get(), filename, ""));

        if (strNumlist->Exists()) {
            if (false ==
                strNumlist->DecodeIfArmored(false))  // bEscapedIsAllowed=true
                                                     // by default.
            {
                LogError()(OT_PRETTY_STATIC(OTCronItem))(
                    "List of recurring transactions; string apparently was "
                    "encoded and then failed decoding. Contents: ")(
                    strNumlist)(".")
                    .Flush();
                return false;
            } else {
                output.Add(strNumlist);
            }
        }
    }

    return true;
}

// static
// Client-side only.
auto OTCronItem::EraseActiveCronReceipt(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const TransactionNumber& lTransactionNum,
    const identifier::Nym& nymID,
    const identifier::Notary& notaryID) -> bool
{
    auto strNotaryID = String::Factory(notaryID);
    auto filename = api::Legacy::GetFilenameCrn(lTransactionNum);

    const char* szFoldername = api.Internal().Legacy().Cron();

    // Before we remove the cron item receipt itself, first we need to load up
    // the local list of active (recurring) transactions, and remove the number
    // from that list. Otherwise the GUI will continue thinking the transaction
    // is active in cron.
    //
    auto list_filename = api::Legacy::GetFilenameLst(nymID.str());

    if (OTDB::Exists(
            api,
            dataFolder,
            szFoldername,
            strNotaryID->Get(),
            list_filename,
            "")) {
        // Load up existing list, to remove the transaction num from it.
        //
        NumList numlist;

        auto strNumlist = String::Factory(OTDB::QueryPlainString(
            api,
            dataFolder,
            szFoldername,
            strNotaryID->Get(),
            list_filename,
            ""));

        if (strNumlist->Exists()) {
            if (false ==
                strNumlist->DecodeIfArmored(false))  // bEscapedIsAllowed=true
                                                     // by default.
            {
                LogError()(OT_PRETTY_STATIC(OTCronItem))(
                    "List of recurring transactions; string apparently "
                    "was encoded "
                    "and then failed decoding. Contents: ")(strNumlist)(".")
                    .Flush();
            } else {
                numlist.Add(strNumlist);
            }
        }

        strNumlist->Release();

        if (numlist.Count() > 0) { numlist.Remove(lTransactionNum); }

        if (0 == numlist.Count()) {
            if (!OTDB::EraseValueByKey(
                    api,
                    dataFolder,
                    szFoldername,
                    strNotaryID->Get(),
                    list_filename,
                    "")) {
                LogConsole()(OT_PRETTY_STATIC(OTCronItem))(
                    "FYI, failure erasing recurring IDs file: ")(
                    szFoldername)(api::Legacy::PathSeparator())(
                    strNotaryID)(api::Legacy::PathSeparator())(
                    list_filename)(".")
                    .Flush();
            }
        } else {
            numlist.Output(strNumlist);

            auto strFinal = String::Factory();
            auto ascTemp = Armored::Factory(strNumlist);

            if (false == ascTemp->WriteArmoredString(
                             strFinal, "ACTIVE CRON ITEMS"))  // todo hardcoding
            {
                LogError()(OT_PRETTY_STATIC(OTCronItem))(
                    "Error re-saving recurring IDs (failed writing "
                    "armored string): ")(
                    szFoldername)(api::Legacy::PathSeparator())(
                    strNotaryID)(api::Legacy::PathSeparator())(
                    list_filename)(".")
                    .Flush();
                return false;
            } else {
                bool bSaved = OTDB::StorePlainString(
                    api,
                    strFinal->Get(),
                    dataFolder,
                    szFoldername,
                    strNotaryID->Get(),
                    list_filename,
                    "");

                if (!bSaved) {
                    LogError()(OT_PRETTY_STATIC(OTCronItem))(
                        "Error re-saving recurring IDs: ")(
                        szFoldername)(api::Legacy::PathSeparator())(
                        strNotaryID)(api::Legacy::PathSeparator())(
                        list_filename)(".")
                        .Flush();
                    return false;
                }
            }
        }
    }

    // Now that the list is updated, let's go ahead and erase the actual cron
    // item itself.
    //
    if (!OTDB::Exists(
            api, dataFolder, szFoldername, strNotaryID->Get(), filename, "")) {
        LogError()(OT_PRETTY_STATIC(OTCronItem))("File does not exist: ")(
            szFoldername)(api::Legacy::PathSeparator())(
            strNotaryID)(api::Legacy::PathSeparator())(filename.c_str())(".")
            .Flush();
        return false;
    }

    if (!OTDB::EraseValueByKey(
            api, dataFolder, szFoldername, strNotaryID->Get(), filename, "")) {
        LogError()(OT_PRETTY_STATIC(OTCronItem))("Error erasing file: ")(
            szFoldername)(api::Legacy::PathSeparator())(
            strNotaryID)(api::Legacy::PathSeparator())(filename.c_str())(".")
            .Flush();
        return false;
    }

    return true;
}

auto OTCronItem::SaveActiveCronReceipt(const identifier::Nym& theNymID)
    -> bool  // Client-side
             // only.
{
    const std::int64_t lOpeningNum = GetOpeningNumber(theNymID);

    auto strNotaryID = String::Factory(GetNotaryID());
    auto filename =
        api::Legacy::GetFilenameCrn(lOpeningNum);  // cron/TRANSACTION_NUM.crn

    const char* szFoldername = api_.Internal().Legacy().Cron();  // cron

    if (OTDB::Exists(
            api_,
            api_.DataFolder(),
            szFoldername,
            strNotaryID->Get(),
            filename,
            "")) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Cron Record already exists for transaction ")(GetTransactionNum())(
            " ")(szFoldername)(api::Legacy::PathSeparator())(
            strNotaryID)(api::Legacy::PathSeparator())(filename)(", ")(
            "overwriting. ")
            .Flush();
        // NOTE: We could just return here. But what if the record we have is
        // corrupted somehow?
        // Might as well just write it there again, so I let this continue
        // running.
    } else  // It wasn't there already, so we need to save the number in our
            // local list of trans nums.
    {
        auto list_filename = api::Legacy::GetFilenameLst(theNymID.str());
        NumList numlist;

        if (OTDB::Exists(
                api_,
                api_.DataFolder(),
                szFoldername,
                strNotaryID->Get(),
                list_filename,
                "")) {
            // Load up existing list, to add the new transaction num to it.
            //
            auto strNumlist = String::Factory(OTDB::QueryPlainString(
                api_,
                api_.DataFolder(),
                szFoldername,
                strNotaryID->Get(),
                list_filename,
                ""));

            if (strNumlist->Exists()) {
                if (false == strNumlist->DecodeIfArmored(
                                 false))  // bEscapedIsAllowed=true
                                          // by default.
                {
                    LogError()(OT_PRETTY_CLASS())(
                        "Input string apparently was encoded and then"
                        " failed decoding. Contents: ")(strNumlist)(".")
                        .Flush();
                } else {
                    numlist.Add(strNumlist);
                }
            }
        }

        numlist.Add(lOpeningNum);

        auto strNumlist = String::Factory();

        if (numlist.Output(strNumlist)) {
            auto strFinal = String::Factory();
            auto ascTemp = Armored::Factory(strNumlist);

            if (false == ascTemp->WriteArmoredString(
                             strFinal, "ACTIVE CRON ITEMS"))  // todo hardcoding
            {
                LogError()(OT_PRETTY_CLASS())(
                    "Error saving recurring IDs (failed writing armored "
                    "string): ")(szFoldername)(api::Legacy::PathSeparator())(
                    strNotaryID)(api::Legacy::PathSeparator())(
                    list_filename)(".")
                    .Flush();
                return false;
            }

            bool bSaved = OTDB::StorePlainString(
                api_,
                strFinal->Get(),
                api_.DataFolder(),
                szFoldername,
                strNotaryID->Get(),
                list_filename,
                "");

            if (!bSaved) {
                LogError()(OT_PRETTY_CLASS())("Error saving recurring IDs: ")(
                    szFoldername)(api::Legacy::PathSeparator())(
                    strNotaryID)(api::Legacy::PathSeparator())(
                    list_filename)(".")
                    .Flush();
                return false;
            }
        }
    }

    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(m_strRawFile);

    if (false ==
        ascTemp->WriteArmoredString(strFinal, m_strContractType->Get())) {
        LogError()(OT_PRETTY_CLASS())(
            "Error saving file (failed writing armored string): ")(
            szFoldername)(api::Legacy::PathSeparator())(
            strNotaryID)(api::Legacy::PathSeparator())(filename)("")
            .Flush();
        return false;
    }

    bool bSaved = OTDB::StorePlainString(
        api_,
        strFinal->Get(),
        api_.DataFolder(),
        szFoldername,
        strNotaryID->Get(),
        filename,
        "");

    if (!bSaved) {
        LogError()(OT_PRETTY_CLASS())("Error saving file: ")(
            szFoldername)(api::Legacy::PathSeparator())(
            strNotaryID)(api::Legacy::PathSeparator())(filename)(".")
            .Flush();
        return false;
    }

    return bSaved;
}

// When first adding anything to Cron, a copy needs to be saved in a folder
// somewhere.
// (Just for our records.) For example, before I start updating the status on
// any Trade,
// I have already saved the user's original Trade object (from his request) to a
// folder.
// Now I have the freedom to ReleaseSignatures on the Trade and re-sign it with
// the
// server's Nym as it updates over time.  The user cannot challenge the Trade
// because
// the server has the original copy on file and sends it with all receipts.

auto OTCronItem::SaveCronReceipt() -> bool
{
    auto filename = api::Legacy::GetFilenameCrn(
        GetTransactionNum());  // cron/TRANSACTION_NUM.crn
    const char* szFoldername = api_.Internal().Legacy().Cron();  // cron

    if (OTDB::Exists(api_, api_.DataFolder(), szFoldername, filename, "", "")) {
        LogError()(OT_PRETTY_CLASS())(
            "Cron Record already exists for transaction ")(GetTransactionNum())(
            " ")(szFoldername)(api::Legacy::PathSeparator())(
            filename)(", yet inexplicably attempted to record it again.")
            .Flush();
        return false;
    }

    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(m_strRawFile);

    if (false ==
        ascTemp->WriteArmoredString(strFinal, m_strContractType->Get())) {
        LogError()(OT_PRETTY_CLASS())(
            "Error saving file (failed writing armored string): ")(
            szFoldername)(api::Legacy::PathSeparator())(filename)(".")
            .Flush();
        return false;
    }

    bool bSaved = OTDB::StorePlainString(
        api_,
        strFinal->Get(),
        api_.DataFolder(),
        szFoldername,
        filename,
        "",
        "");

    if (!bSaved) {
        LogError()(OT_PRETTY_CLASS())("Error saving file: ")(
            szFoldername)(api::Legacy::PathSeparator())(filename)(".")
            .Flush();
        return false;
    }

    return bSaved;
}

auto OTCronItem::SetDateRange(const Time VALID_FROM, const Time VALID_TO)
    -> bool
{
    const auto CURRENT_TIME = Clock::now();
    SetCreationDate(CURRENT_TIME);

    // VALID_FROM
    //
    // The default "valid from" time is NOW.
    if (Time{} >= VALID_FROM) {
        SetValidFrom(CURRENT_TIME);
    } else {
        SetValidFrom(VALID_FROM);
    }

    // VALID_TO
    //
    // The default "valid to" time is 0 (which means no expiration date / cancel
    // anytime.)
    if (Time{} == VALID_TO)  // VALID_TO is 0
    {
        SetValidTo(Time{});        // Keep it at zero then, so it
                                   // won't expire.
    } else if (Time{} < VALID_TO)  // VALID_TO is ABOVE zero...
    {
        if (VALID_TO < VALID_FROM)  // If Valid-To date is EARLIER than
                                    // Valid-From date...
        {
            LogError()(OT_PRETTY_CLASS())("VALID_TO (")(
                VALID_TO)(") is earlier than VALID_FROM (")(VALID_FROM)(").")
                .Flush();
            return false;
        }

        SetValidTo(VALID_TO);  // Set it to whatever it is,
                               // since it is now validated as
                               // higher than Valid-From.
    } else                     // VALID_TO is a NEGATIVE number... Error.
    {
        LogError()(OT_PRETTY_CLASS())("Negative value for valid_to: ")(VALID_TO)
            .Flush();

        return false;
    }

    return true;
}
// These are for finalReceipt
// The Cron Item stores a list of these closing transaction numbers,
// used for closing a transaction.
//
auto OTCronItem::GetCountClosingNumbers() const -> std::int32_t
{
    return static_cast<std::int32_t>(m_dequeClosingNumbers.size());
}

auto OTCronItem::GetClosingTransactionNoAt(std::uint32_t nIndex) const
    -> std::int64_t
{
    if (m_dequeClosingNumbers.size() <= nIndex) {
        LogError()(OT_PRETTY_CLASS())(
            "nIndex"
            " is equal or larger than m_dequeClosingNumbers.size()!")
            .Flush();
        OT_FAIL;
    }

    return m_dequeClosingNumbers.at(nIndex);
}

void OTCronItem::AddClosingTransactionNo(
    const std::int64_t& lClosingTransactionNo)
{
    m_dequeClosingNumbers.push_back(lClosingTransactionNo);
}

/// See if theNym has rights to remove this item from Cron.
auto OTCronItem::CanRemoveItemFromCron(const otx::context::Client& context)
    -> bool
{
    const auto strNotaryID = String::Factory(GetNotaryID());

    // You don't just go willy-nilly and remove a cron item from a market
    // unless you check first and make sure the Nym who requested it
    // actually has said number (or a related closing number) signed out to
    // him on his last receipt...
    if (!context.Nym()->CompareID(GetSenderNymID())) {
        LogInsane()(OT_PRETTY_CLASS())(
            "theNym is not the originator of this CronItem. (He could be "
            "a "
            "recipient though, so this is normal.)")
            .Flush();

        return false;
    }
    // By this point, that means theNym is DEFINITELY the originator
    // (sender)...
    else if (GetCountClosingNumbers() < 1) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Weird: Sender tried to remove a cron item; expected at "
            "least "
            "1 closing number to be available"
            " -- that wasn't. (Found ")(GetCountClosingNumbers())(").")
            .Flush();

        return false;
    }

    if (!context.VerifyIssuedNumber(GetClosingNum())) {
        LogConsole()(OT_PRETTY_CLASS())("Closing number didn't "
                                        "verify (for removal from cron).")
            .Flush();

        return false;
    }

    // By this point, we KNOW theNym is the sender, and we KNOW there are
    // the proper number of transaction numbers available to close. We also
    // know that this cron item really was on the cron object, since that is
    // where it was looked up from, when this function got called! So I'm
    // pretty sure, at this point, to authorize removal, as long as the
    // transaction num is still issued to theNym (this check here.)

    return context.VerifyIssuedNumber(GetOpeningNum());

    // Normally this will be all we need to check. The originator will have
    // the transaction number signed-out to him still, if he is trying to
    // close it. BUT--in some cases, someone who is NOT the originator can
    // cancel. Like in a payment plan, the sender is also the depositor, who
    // would normally be the person cancelling the plan. But technically,
    // the RECIPIENT should also have the ability to cancel that payment
    // plan.  BUT: the transaction number isn't signed out to the
    // RECIPIENT... In THAT case, the below VerifyIssuedNum() won't work! In
    // those cases, expect that the special code will be in the subclasses
    // override of this function. (OTPaymentPlan::CanRemoveItem() etc)

    // P.S. If you override this function, maybe call the parent
    // (OTCronItem::CanRemoveItem) first, for the VerifyIssuedNum call
    // above. Only if that fails, do you need to dig deeper...
}

// OTCron calls this regularly, which is my chance to expire, etc.
// Child classes will override this, AND call it (to verify valid date
// range.)
//
// Return False:    REMOVE this Cron Item from Cron.
// Return True:        KEEP this Cron Item on Cron (for now.)
//
auto OTCronItem::ProcessCron(const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nullptr != m_pCron);

    if (IsFlaggedForRemoval()) {
        LogDebug()(OT_PRETTY_CLASS())("Flagged for removal: ")(
            m_strContractType)
            .Flush();
        return false;
    }

    // I call IsExpired() here instead of VerifyCurrentDate(). The Cron Item
    // will stay on
    // Cron even if it is NOT YET valid. But once it actually expires, this
    // will remove it.
    if (IsExpired()) {
        LogDebug()(OT_PRETTY_CLASS())("Expired ")(m_strContractType).Flush();
        return false;
    }

    // As far as this code is concerned, the item can stay on cron for now.
    // Return true.
    return true;
}

// OTCron calls this when a cron item is added.
// bForTheFirstTime=true means that this cron item is being
// activated for the very first time. (Versus being re-added
// to cron after a server reboot.)
//
void OTCronItem::HookActivationOnCron(
    const PasswordPrompt& reason,
    bool bForTheFirstTime)
{
    // Put anything else in here, that needs to be done in the
    // cron item base class, upon activation. (This executes
    // no matter what, even if onActivate() is overridden.)

    if (bForTheFirstTime) {
        onActivate(reason);  // Subclasses may override this.
    }
    //
    // MOST NOTABLY,
    // OTSmartContract overrides this, so it can allow the SCRIPT
    // a chance to hook onActivate() as well.
}

// OTCron calls this when a cron item is removed
// This gives each item a chance to drop a final receipt,
// and clean up any memory, before being destroyed.
//
void OTCronItem::HookRemovalFromCron(
    const api::session::Wallet& wallet,
    Nym_p pRemover,
    std::int64_t newTransactionNo,
    const PasswordPrompt& reason)
{
    auto pServerNym = serverNym_;
    OT_ASSERT(nullptr != pServerNym);

    // Generate new transaction number for these new inbox receipts.
    const std::int64_t lNewTransactionNumber = newTransactionNo;

    //    OT_ASSERT(lNewTransactionNumber > 0); // this can be my reminder.
    if (0 == lNewTransactionNumber) {
        LogError()(OT_PRETTY_CLASS())("** ERROR! Final receipt not "
                                      "added to inbox since no "
                                      "transaction numbers were available!")
            .Flush();
    } else {
        // Everytime a payment processes, or a trade, then a receipt is put
        // in the user's inbox. This contains a copy of the current payment
        // or trade (which took money from the user's acct.)
        //
        // ==> So I increment the payment count each time before dropping
        // the receipt. (I also use a fresh transaction number when I put it
        // into the inbox.) That way, the user will never get the same
        // receipt for the same plan twice. It cannot take funds from his
        // account, without a new payment
        // count and a new transaction number on a new receipt. Until the
        // user accepts the receipt out of his inbox with a new balance
        // agreement, the existing receipts can be added up and compared to
        // the last balance agreement, to verify the current balance. Every
        // receipt from a processing
        // payment will have the user's authorization, signature, and terms,
        // as well as the update in balances due to the payment, signed by
        // the server.

        // In the case of the FINAL RECEIPT, I do NOT increment the count,
        // so you can see it will have the same payment count as the last
        // paymentReceipt. (if there were 5 paymentReceipts, from 1 to 5,
        // then the finalReceipt will also be 5. This is evidence of what
        // the last paymentReceipt WAS.)

        // The TRANSACTION will be dropped into the INBOX with "In Reference
        // To" information, containing the ORIGINAL SIGNED REQUEST.
        //
        std::unique_ptr<OTCronItem> pOrigCronItem =
            OTCronItem::LoadCronReceipt(api_, GetTransactionNum());
        // OTCronItem::LoadCronReceipt loads the original version with the
        // user's signature.
        // (Updated versions, as processing occurs, are signed by the
        // server.)
        OT_ASSERT(false != bool(pOrigCronItem));

        // Note: elsewhere, we verify the Nym's signature. But in this
        // place, we verify the SERVER's signature. (The server signed the
        // cron receipt just before it was first saved, so it has two
        // signatures on it.)
        //
        {
            bool bValidSignture = pOrigCronItem->VerifySignature(*pServerNym);
            if (!bValidSignture) {
                LogError()(OT_PRETTY_CLASS())("Failure verifying signature of "
                                              "server on Cron Item!")
                    .Flush();
                OT_FAIL;
            }
        }

        // I now have a String copy of the original CronItem...
        const auto strOrigCronItem = String::Factory(*pOrigCronItem);

        // The Nym who is actively requesting to remove a cron item will be
        // passed in as pRemover.
        // However, sometimes there is no Nym... perhaps it just expired and
        // pRemover is nullptr.
        // The originating Nym (if different than remover) is loaded up.
        // Otherwise the originator
        // pointer just pointers to *pRemover.
        //
        Nym_p pOriginator = nullptr;

        if (pServerNym->CompareID(pOrigCronItem->GetSenderNymID())) {
            pOriginator = pServerNym;  // Just in case the originator Nym is
                                       // also the server Nym.
        }  // This MIGHT be unnecessary, since pRemover is(I think) already
           // transmogrified
        // ******************************************************* to
        // pServer earlier, if they share the same ID.
        //
        // If pRemover is NOT nullptr, and he has the Originator's ID...
        // then set the pointer accordingly.
        //
        else if (
            (nullptr != pRemover) &&
            (true == pRemover->CompareID(pOrigCronItem->GetSenderNymID()))) {
            pOriginator = pRemover;  // <======== now both pointers are set
                                     // (to same Nym). DONE!
        }

        // At this point, pRemover MIGHT be set, or nullptr. (And that's
        // that -- pRemover may always be nullptr.)
        //
        // if pRemover IS set, then pOriginator MIGHT point to it as well.
        // (If the IDs match. Done above.) pOriginator might also still be
        // nullptr. (If pRemover is nullptr, then pOriginator DEFINITELY
        // is.) pRemover is loaded (or not). Next let's make SURE
        // pOriginator is loaded, if it wasn't already...
        //
        if (nullptr == pOriginator) {
            // GetSenderNymID() should be the same on THIS (updated version
            // of the same cron item) but for whatever reason, I'm checking
            // the nymID on the original version. Sue me.
            //
            const OTNymID NYM_ID = pOrigCronItem->GetSenderNymID();
            pOriginator = api_.Wallet().Nym(NYM_ID);
        }

        // pOriginator should NEVER be nullptr by this point, unless there
        // was an ERROR in the above block. We even loaded the guy from
        // storage, if we had to.
        //
        if (nullptr != pOriginator) {
            // Drop the FINAL RECEIPT(s) into the user's inbox(es)!!
            // Pass in strOrigCronItem and lNewTransactionNumber which were
            // obtained above.
            //
            onFinalReceipt(
                *pOrigCronItem,
                lNewTransactionNumber,
                pOriginator,
                pRemover,
                reason);
        } else {
            LogError()(OT_PRETTY_CLASS())(
                "MAJOR ERROR in OTCronItem::HookRemovalFromCron!! Failed "
                "loading Originator Nym for Cron Item.")
                .Flush();
        }
    }

    // Remove corresponding offer from market, if applicable.
    //
    onRemovalFromCron(reason);
}

// This function is overridden in OTTrade, OTAgreement, and OTSmartContract.
//
// I'm put a default implementation here "Just Because."
//
// This is called by HookRemovalFromCron().
//
void OTCronItem::onFinalReceipt(
    OTCronItem& theOrigCronItem,
    const std::int64_t& lNewTransactionNumber,
    Nym_p theOriginator,
    Nym_p pRemover,
    const PasswordPrompt& reason)
{
    OT_ASSERT(nullptr != serverNym_);

    auto context = api_.Wallet().Internal().mutable_ClientContext(
        theOriginator->ID(), reason);

    // The finalReceipt Item's ATTACHMENT contains the UPDATED Cron Item.
    // (With the SERVER's signature on it!)
    //
    auto strUpdatedCronItem = String::Factory(*this);
    OTString pstrAttachment = strUpdatedCronItem;

    const auto strOrigCronItem = String::Factory(theOrigCronItem);

    // First, we are closing the transaction number ITSELF, of this cron
    // item, as an active issued number on the originating nym. (Changing it
    // to CLOSED.)
    //
    // Second, we're verifying the CLOSING number, and using it as the
    // closing number on the FINAL RECEIPT (with that receipt being
    // "InReferenceTo" GetTransactionNum())
    const TransactionNumber lOpeningNumber = theOrigCronItem.GetOpeningNum();
    const TransactionNumber lClosingNumber = theOrigCronItem.GetClosingNum();

    // I'm ASSUMING here that pRemover is also theOriginator.
    //
    // REMEMBER: Most subclasses will override this method, and THEY
    // are the cases where pRemover is someone other than theOriginator.
    // That's why they have a different version of onFinalReceipt.
    if ((lOpeningNumber > 0) &&
        context.get().VerifyIssuedNumber(lOpeningNumber)) {
        // The Nym (server side) stores a list of all opening and closing
        // cron #s. So when the number is released from the Nym, we also
        // take it off that list.
        context.get().CloseCronItem(lOpeningNumber);
        context.get().ConsumeIssued(lOpeningNumber);

        // the RemoveIssued call means the original transaction# (to find
        // this cron item on cron) is now CLOSED. But the Transaction itself
        // is still OPEN. How? Because the CLOSING number is still signed
        // out. The closing number is also USED, since the
        // NotarizePaymentPlan or NotarizeMarketOffer call, but it remains
        // ISSUED, until the final receipt itself is accepted during a
        // process inbox.
        //

        if (!DropFinalReceiptToNymbox(
                GetSenderNymID(),
                lNewTransactionNumber,
                strOrigCronItem,
                GetOriginType(),
                reason,
                String::Factory(),  // note
                pstrAttachment)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failure dropping finalReceipt to Nymbox.")
                .Flush();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed doing "
                                      "VerifyIssuedNum(theOrigCronItem."
                                      "GetTransactionNum()).")
            .Flush();
    }

    if ((lClosingNumber > 0) &&
        context.get().VerifyIssuedNumber(lClosingNumber)) {
        // SENDER only. (CronItem has no recipient. That's in the subclass.)
        if (!DropFinalReceiptToInbox(
                GetSenderNymID(),
                GetSenderAcctID(),
                lNewTransactionNumber,
                lClosingNumber,  // The closing transaction number to
                                 // put on the receipt.
                strOrigCronItem,
                GetOriginType(),
                reason,
                String::Factory(),  // note
                pstrAttachment)) {  // pActualAcct = nullptr by default.
                                    // (This call will load it up in order
                                    // to update the inbox hash.)
            LogError()(OT_PRETTY_CLASS())(
                "Failure dropping receipt into inbox.")
                .Flush();
        }

        // In this case, I'm passing nullptr for pstrNote, since there is no
        // note.
        // (Additional information would normally be stored in the note.)

        // This part below doesn't happen until you ACCEPT the final receipt
        // (when processing your inbox.)
        //
        //      theOriginator.RemoveIssuedNum(strNotaryID, lClosingNumber,
        // true); //bSave=false
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Failed verifying "
            "lClosingNumber=theOrigCronItem.GetClosingTransactionNoAt(0)>"
            "0 && "
            "theOriginator.VerifyTransactionNum(lClosingNumber).")
            .Flush();
    }

    // QUESTION: Won't there be Cron Items that have no asset account at
    // all? In which case, there'd be no need to drop a final receipt, but I
    // don't think that's the case, since you have to use a transaction
    // number to get onto cron in the first place.
}

// This is the "DROPS FINAL RECEIPT" function.
// "Final Receipts" are used by Cron Items, as the last receipt for a given
// transaction number.
//
auto OTCronItem::DropFinalReceiptToInbox(
    const identifier::Nym& NYM_ID,
    const Identifier& ACCOUNT_ID,
    const std::int64_t& lNewTransactionNumber,
    const std::int64_t& lClosingNumber,
    const String& strOrigCronItem,
    const originType theOriginType,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment) -> bool
{
    OT_ASSERT(nullptr != serverNym_);

    const identity::Nym& pServerNym = *serverNym_;
    // Load the inbox in case it already exists.
    auto theInbox{api_.Factory().InternalSession().Ledger(
        NYM_ID, ACCOUNT_ID, GetNotaryID())};

    OT_ASSERT(false != bool(theInbox));

    // Inbox will receive notification of something ALREADY DONE.
    bool bSuccessLoading = theInbox->LoadInbox();

    // ...or generate it otherwise...

    if (true == bSuccessLoading) {
        bSuccessLoading = theInbox->VerifyAccount(pServerNym);
    } else {
        LogError()(OT_PRETTY_CLASS())("ERROR loading inbox ledger.").Flush();
    }
    //      otErr << szFunc << ": ERROR loading inbox ledger.\n";
    //  else
    //      bSuccessLoading = theInbox->GenerateLedger(ACCOUNT_ID,
    //      GetNotaryID(), OTLedger::inbox, true); // bGenerateFile=true

    if (!bSuccessLoading) {
        LogError()(OT_PRETTY_CLASS())(
            "ERROR loading or generating an inbox. (FAILED "
            "WRITING RECEIPT!!).")
            .Flush();
        return false;
    } else {
        // Start generating the receipts

        auto pTrans1{api_.Factory().InternalSession().Transaction(
            *theInbox,
            transactionType::finalReceipt,
            theOriginType,
            lNewTransactionNumber)};

        // The inbox will get a receipt with the new transaction ID.
        // That receipt has an "in reference to" field containing the
        // original cron item.

        OT_ASSERT(false != bool(pTrans1));

        // set up the transaction items (each transaction may have multiple
        // items... but not in this case.)
        auto pItem1{api_.Factory().InternalSession().Item(
            *pTrans1, itemType::finalReceipt, api_.Factory().Identifier())};

        OT_ASSERT(false != bool(pItem1));

        pItem1->SetStatus(Item::acknowledgement);

        //
        // Here I make sure that the receipt (the inbox notice) references
        // the transaction number that the trader originally used to issue
        // the cron item... This number is used to match up offers to
        // trades, and used to track all cron items. (All Cron items require
        // a transaction from the user to add them to Cron in the first
        // place.)
        //
        const std::int64_t lOpeningNum = GetOpeningNumber(NYM_ID);

        pTrans1->SetReferenceToNum(lOpeningNum);
        pTrans1->SetNumberOfOrigin(lOpeningNum);
        //      pItem1-> SetReferenceToNum(lOpeningNum);

        // The reference on the transaction contains an OTCronItem, in this
        // case.
        // The original cron item, versus the updated cron item (which is
        // stored on the finalReceipt item just below here.)
        //
        pTrans1->SetReferenceString(strOrigCronItem);

        pTrans1->SetClosingNum(lClosingNumber);  // This transaction is the
                                                 // finalReceipt for
                                                 // GetTransactionNum(), as
                                                 // lClosingNumber.
        //      pItem1-> SetClosingNum(lClosingNumber);
        //
        // NOTE: I COULD look up the closing number by doing a call to
        // GetClosingNumber(ACCOUNT_ID);
        // But that is already taken care of where it matters, and passed in
        // here properly already, so it
        // would be superfluous.

        // The finalReceipt ITEM's NOTE contains the UPDATED CRON ITEM.
        //
        if (pstrNote->Exists()) {
            pItem1->SetNote(pstrNote);  // in markets, this is updated
                                        // trade.
        }

        // Also set the ** UPDATED OFFER ** as the ATTACHMENT on the **
        // item.** (With the SERVER's signature on it!) // in markets, this
        // is updated offer.
        //
        if (pstrAttachment->Exists()) { pItem1->SetAttachment(pstrAttachment); }

        // sign the item

        pItem1->SignContract(pServerNym, reason);
        pItem1->SaveContract();

        std::shared_ptr<Item> item1{pItem1.release()};
        pTrans1->AddItem(item1);

        pTrans1->SignContract(pServerNym, reason);
        pTrans1->SaveContract();

        // Here the transaction we just created is actually added to the
        // ledger.
        std::shared_ptr<OTTransaction> trans1{pTrans1.release()};
        theInbox->AddTransaction(trans1);

        // Release any signatures that were there before (They won't
        // verify anymore anyway, since the content has changed.)
        theInbox->ReleaseSignatures();

        // Sign and save.
        theInbox->SignContract(pServerNym, reason);
        theInbox->SaveContract();

        // TODO: Better rollback capabilities in case of failures here:
        auto account =
            api_.Wallet().Internal().mutable_Account(ACCOUNT_ID, reason);

        // Save inbox to storage. (File, DB, wherever it goes.)
        if (account) {
            OT_ASSERT(ACCOUNT_ID == account.get().GetPurportedAccountID());

            if (account.get().SaveInbox(
                    *theInbox, api_.Factory().Identifier())) {
                account.Release();  // inbox hash has changed here, so we
                                    // save the account to reflect that
                                    // change.
            } else {
                account.Abort();
                LogError()(OT_PRETTY_CLASS())(
                    "Failed: account.get().VerifyAccount(*pServerNym).")
                    .Flush();
            }
        } else  // todo: would the account EVER be null here? Should never
                // be. Therefore should we save the inbox here?
        {
            theInbox->SaveInbox(api_.Factory().Identifier());
        }

        // Notice above, if the account loads but fails to verify, then we
        // do not save the Inbox. Todo: ponder wisdom of that decision.

        // Corresponds to the AddTransaction() just above.
        // Details are stored in separate file these days.
        //
        trans1->SaveBoxReceipt(*theInbox);

        return true;  // Really this true should be predicated on ALL the
                      // above functions returning true. Right?
    }                 // ...Right?
}

// Done: IF ACTUAL NYM is NOT passed below, then need to LOAD HIM UP (so we
// can update his NymboxHash after we update the Nymbox.)

// The final receipt doesn't have a closing number in the Nymbox, only in
// the Inbox. That's because in the Nymbox, it's just a notice, and it's not
// there to enforce anything. If you get one in your Nymbox, it's just so
// that you know to remove its "in ref to" number (the opening number) from
// your issued list (so your balance agreements will work :P)
//
auto OTCronItem::DropFinalReceiptToNymbox(
    const identifier::Nym& NYM_ID,
    const TransactionNumber& lNewTransactionNumber,
    const String& strOrigCronItem,
    const originType theOriginType,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment) -> bool
{
    OT_ASSERT(nullptr != serverNym_);

    Nym_p pServerNym(serverNym_);

    auto theLedger{
        api_.Factory().InternalSession().Ledger(NYM_ID, NYM_ID, GetNotaryID())};

    OT_ASSERT(false != bool(theLedger));

    // Inbox will receive notification of something ALREADY DONE.
    bool bSuccessLoading = theLedger->LoadNymbox();

    // ...or generate it otherwise...

    if (true == bSuccessLoading) {
        bSuccessLoading = theLedger->VerifyAccount(*pServerNym);
    } else {
        LogError()(OT_PRETTY_CLASS())("Unable to load Nymbox.").Flush();
    }
    //    else
    //        bSuccessLoading        = theLedger->GenerateLedger(NYM_ID,
    // GetNotaryID(), OTLedger::nymbox, true); // bGenerateFile=true

    if (!bSuccessLoading) {
        LogError()(OT_PRETTY_CLASS())(
            "ERROR loading or generating a nymbox. (FAILED "
            "WRITING RECEIPT!!).")
            .Flush();
        return false;
    }

    auto pTransaction{api_.Factory().InternalSession().Transaction(
        *theLedger,
        transactionType::finalReceipt,
        theOriginType,
        lNewTransactionNumber)};

    if (false != bool(pTransaction)) {
        pTransaction->SetOriginType(theOriginType);

        // The nymbox will get a receipt with the new transaction ID.
        // That receipt has an "in reference to" field containing the
        // original cron item.

        // set up the transaction items (each transaction may have multiple
        // items... but not in this case.)
        auto pItem1{api_.Factory().InternalSession().Item(
            *pTransaction,
            itemType::finalReceipt,
            api_.Factory().Identifier())};

        OT_ASSERT(false != bool(pItem1));

        pItem1->SetStatus(Item::acknowledgement);

        const std::int64_t lOpeningNumber = GetOpeningNumber(NYM_ID);

        // Here I make sure that the receipt (the nymbox notice) references
        // the transaction number that the trader originally used to issue
        // the cron item... This number is used to match up offers to
        // trades, and used to track all cron items. (All Cron items require
        // a transaction from the user to add them to Cron in the first
        // place.)

        pTransaction->SetReferenceToNum(lOpeningNumber);  // Notice this
                                                          // same number is
                                                          // set twice
                                                          // (again just
        // below), so might be an opportunity to store
        // something else in one of them.

        // The reference on the transaction contains an OTCronItem, in this
        // case.
        // The original cron item, versus the updated cron item (which is
        // stored on the finalReceipt item just below here.)
        //
        pTransaction->SetReferenceString(strOrigCronItem);

        // Normally in the Inbox, the "Closing Num" is set to the closing
        // number, in reference to the opening number. (on a finalReceipt)
        // But in the NYMBOX, we are sending the Opening Number in that
        // spot. The purpose is so the client side will know not to use that
        // opening number as a valid transaction # in its transaction
        // statements and balance statements, since the number is now gone.
        // Otherwise the Nym wouldn't know any better, and he'd keep signing
        // for it, and therefore his balance agreements would start to fail.

        pTransaction->SetClosingNum(lOpeningNumber);  // This transaction is the
                                                      // finalReceipt for
                                                      // GetTransactionNum().
                                                      // (Which is also the
                                                      // original transaction
                                                      // number.)

        // The finalReceipt ITEM's NOTE contains the UPDATED CRON ITEM.
        //
        if (pstrNote->Exists()) {
            pItem1->SetNote(pstrNote);  // in markets, this is updated
                                        // trade.
        }

        // Also set the ** UPDATED OFFER ** as the ATTACHMENT on the **
        // item.** (With the SERVER's signature on it!) // in markets, this
        // is updated offer.
        //
        if (!pstrAttachment->Exists()) {
            pItem1->SetAttachment(pstrAttachment);
        }

        // sign the item

        pItem1->SignContract(*pServerNym, reason);
        pItem1->SaveContract();

        std::shared_ptr<Item> item1{pItem1.release()};
        pTransaction->AddItem(item1);

        pTransaction->SignContract(*pServerNym, reason);
        pTransaction->SaveContract();

        // Here the transaction we just created is actually added to the
        // ledger.
        std::shared_ptr<OTTransaction> transaction{pTransaction.release()};
        theLedger->AddTransaction(transaction);

        // Release any signatures that were there before (They won't
        // verify anymore anyway, since the content has changed.)
        theLedger->ReleaseSignatures();

        // Sign and save.
        theLedger->SignContract(*pServerNym, reason);
        theLedger->SaveContract();

        // TODO: Better rollback capabilities in case of failures here:

        auto theNymboxHash = api_.Factory().Identifier();

        // Save nymbox to storage. (File, DB, wherever it goes.)
        theLedger->SaveNymbox(theNymboxHash);

        // This corresponds to the AddTransaction() call just above.
        // These are stored in a separate file now.
        //
        transaction->SaveBoxReceipt(*theLedger);

        // Update the NymboxHash (in the nymfile.)
        //
        auto context =
            api_.Wallet().Internal().mutable_ClientContext(NYM_ID, reason);
        context.get().SetLocalNymboxHash(theNymboxHash);

        // Really this true should be predicated on ALL the above functions
        // returning true. Right?

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed trying to create finalReceipt.")
            .Flush();
    }

    return false;  // unreachable.
}

auto OTCronItem::GetOpeningNum() const -> std::int64_t
{
    return GetTransactionNum();
}

auto OTCronItem::GetClosingNum() const -> std::int64_t
{
    return (GetCountClosingNumbers() > 0) ? GetClosingTransactionNoAt(0)
                                          : 0;  // todo stop hardcoding.
}

auto OTCronItem::IsValidOpeningNumber(const std::int64_t& lOpeningNum) const
    -> bool
{
    if (GetOpeningNum() == lOpeningNum) { return true; }

    return false;
}

auto OTCronItem::GetOpeningNumber(const identifier::Nym& theNymID) const
    -> std::int64_t
{
    const auto& theSenderNymID = GetSenderNymID();

    if (theSenderNymID == theNymID) { return GetOpeningNum(); }

    return 0;
}

auto OTCronItem::GetClosingNumber(const Identifier& theAcctID) const
    -> std::int64_t
{
    const auto& theSenderAcctID = GetSenderAcctID();

    if (theAcctID == theSenderAcctID) { return GetClosingNum(); }

    return 0;
}

// You usually wouldn't want to use this, since if the transaction failed,
// the opening number is already burned and gone. But there might be cases
// where it's not, and you want to retrieve it. So I added this function for
// those cases. In most cases, you will prefer HarvestClosingNumbers().
void OTCronItem::HarvestOpeningNumber(otx::context::Server& context)
{
    // The Nym is the original sender. (If Compares true). IN CASES where
    // GetTransactionNum() isn't already burned, we can harvest it here.
    // Subclasses will have to override this function for recipients, etc.
    if (context.Nym()->CompareID(GetSenderNymID())) {
        // This function will only "add it back" if it was really there in
        // the first place. (Verifies it is on issued list first, before
        // adding to available list.)
        context.RecoverAvailableNumber(GetOpeningNum());
    }

    // NOTE: if the message failed (transaction never actually ran) then the
    // sender AND recipient can both reclaim their opening numbers. But if
    // the message SUCCEEDED and the transaction FAILED, then only the
    // recipient can claim his opening number -- the sender's is already
    // burned. So then, what if you mistakenly call this function and pass
    // the sender, when that number is already burned? There's nothing this
    // function can do, because we have no way of telling, from inside here,
    // whether the message succeeded or not, and whether the transaction
    // succeeded or not. Therefore we MUST rely on the CALLER to know this,
    // and to avoid calling this function in the first place, if he's
    // sitting on a sender with a failed transaction.
}

// This is a good default implementation.
// Also, some subclasses override this, but they STILL CALL IT.
void OTCronItem::HarvestClosingNumbers(otx::context::Server& context)
{
    // The Nym is the original sender. (If Compares true).
    // GetTransactionNum() is usually already burned, but we can harvest the
    // closing numbers from the "Closing" list, which is only for the
    // sender's numbers. Subclasses will have to override this function for
    // recipients, etc.
    if (context.Nym()->CompareID(GetSenderNymID())) {
        for (std::int32_t i = 0; i < GetCountClosingNumbers(); i++) {
            // This function will only "add it back" if it was really there
            // in the first place. (Verifies it is on issued list first,
            // before adding to available list.)
            context.RecoverAvailableNumber(GetClosingTransactionNoAt(i));
        }
    }
}

auto OTCronItem::GetCancelerID(identifier::Nym& theOutput) const -> bool
{
    if (!IsCanceled()) {
        theOutput.clear();
        return false;
    }

    theOutput.SetString(m_pCancelerNymID->str());
    return true;
}

// When canceling a cron item before it has been activated, use this.
//
auto OTCronItem::CancelBeforeActivation(
    const identity::Nym& theCancelerNym,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(!m_pCancelerNymID->empty());

    if (IsCanceled()) { return false; }

    m_bCanceled = true;
    m_pCancelerNymID = theCancelerNym.ID();

    ReleaseSignatures();
    SignContract(theCancelerNym, reason);
    SaveContract();

    return true;
}

void OTCronItem::InitCronItem()
{
    m_strContractType->Set("CRONITEM");  // in practice should never appear.
                                         // Child classes will overwrite.
}

void OTCronItem::ClearClosingNumbers() { m_dequeClosingNumbers.clear(); }

void OTCronItem::Release_CronItem()
{
    m_CREATION_DATE = Time{};
    m_LAST_PROCESS_DATE = Time{};
    m_PROCESS_INTERVAL = 1s;

    ClearClosingNumbers();

    m_bRemovalFlag = false;
    m_bCanceled = false;
    m_pCancelerNymID->clear();
}

void OTCronItem::Release()
{
    Release_CronItem();

    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto OTCronItem::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    std::int32_t nReturnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    //

    nReturnVal = ot_super::ProcessXMLNode(xml);

    if (nReturnVal != 0) {  // -1 is error, and 1 is "found it". Either way,
                            // return.
        return nReturnVal;  // 0 means "nothing happened, keep going."
    }

    const auto strNodeName = String::Factory(xml->getNodeName());

    if (strNodeName->Compare("closingTransactionNumber")) {
        auto strClosingNumber =
            String::Factory(xml->getAttributeValue("value"));

        if (strClosingNumber->Exists()) {
            const std::int64_t lClosingNumber = strClosingNumber->ToLong();

            AddClosingTransactionNo(lClosingNumber);
        } else {
            LogError()(OT_PRETTY_CLASS())(
                "Error in OTCronItem::ProcessXMLNode: "
                "closingTransactionNumber field without value.")
                .Flush();
            return (-1);  // error condition
        }

        nReturnVal = 1;
    }

    return nReturnVal;
}

void OTCronItem::setNotaryID(const identifier::Notary& notaryID)
{
    notaryID_ = notaryID;
}

OTCronItem::~OTCronItem() { Release_CronItem(); }
}  // namespace opentxs
