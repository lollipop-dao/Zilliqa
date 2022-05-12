/*
 * Copyright (C) 2019 Zilliqa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "MessengerAccountStoreTrie.h"
#include <unordered_map>
#include "libData/AccountData/AccountStoreBase.h"
#include "libMessage/ZilliqaMessage.pb.h"
#include "libUtils/Logger.h"

using namespace boost::multiprecision;
using namespace std;
using namespace ZilliqaMessage;

template <class T = ProtoAccountStore>
bool SerializeToArray(const T& protoMessage, bytes& dst,
                      const unsigned int offset);
bool AccountToProtobuf(const Account& account, ProtoAccount& protoAccount);
bool ProtobufToAccount(const ProtoAccount& protoAccount, Account& account,
                       const Address& addr);

bool MessengerAccountStoreTrie::SetAccountStoreTrie(
    bytes& dst, const unsigned int offset,
    const dev::GenericTrieDB<TraceableDB>& stateTrie,
    const AccountStoreBase& accountStore) {
  ProtoAccountStore result;

  for (const auto& i : stateTrie) {
    ProtoAccountStore::AddressAccount* protoEntry = result.add_entries();
    Address address(i.first);
    protoEntry->set_address(address.data(), address.size);
    ProtoAccount* protoEntryAccount = protoEntry->mutable_account();

    const Account* account = accountStore.GetAccount(address);
    if (account != nullptr) {
      if (!AccountToProtobuf(*account, *protoEntryAccount)) {
        LOG_GENERAL(WARNING, "AccountToProtobuf failed");
        return false;
      }
    } else {
      Account account;
      if (!account.DeserializeBase(bytes(i.second.begin(), i.second.end()),
                                   0)) {
        LOG_GENERAL(WARNING, "Account::DeserializeBase failed");
        continue;
      }
      if (account.GetCodeHash() != dev::h256()) {
        account.SetAddress(address);
      }
      if (!AccountToProtobuf(account, *protoEntryAccount)) {
        LOG_GENERAL(WARNING, "AccountToProtobuf failed");
        return false;
      }
    }

    if (!protoEntryAccount->IsInitialized()) {
      LOG_GENERAL(WARNING, "ProtoAccount initialization failed.");
      return false;
    }
  }

  if (!result.IsInitialized()) {
    LOG_GENERAL(WARNING, "ProtoAccountStore initialization failed.");
    return false;
  }

  return SerializeToArray(result, dst, offset);
}
