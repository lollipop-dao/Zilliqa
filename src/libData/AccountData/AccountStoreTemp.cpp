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

#include "AccountStore.h"
#include "libMessage/Messenger.h"

using namespace std;
using namespace boost::multiprecision;

AccountStoreTemp::AccountStoreTemp(AccountStore& parent) : m_parent(parent) {}

Account* AccountStoreTemp::GetAccount(const Address& address) {
  Account* account =
      AccountStoreBase<map<Address, Account>>::GetAccount(address);
  if (account != nullptr) {
    LOG_GENERAL(INFO, "From AccountStoreBase -> Address: "
                          << address.hex() << "Nonce: " << account->GetNonce());
    return account;
  }

  account = m_parent.GetAccount(address);
  if (account) {
    Account newaccount(*account);
    LOG_GENERAL(INFO, "From AccountStoreTrie -> Address: "
                          << address.hex()
                          << "Nonce: " << newaccount.GetNonce());
    m_addressToAccount->insert(make_pair(address, newaccount));
    return &(m_addressToAccount->find(address))->second;
  }

  return nullptr;
}

bool AccountStoreTemp::DeserializeDelta(const bytes& src, unsigned int offset) {
  LOG_MARKER();

  if (!Messenger::GetAccountStoreDelta(src, offset, *this, true)) {
    LOG_GENERAL(WARNING, "Messenger::GetAccountStoreDelta failed.");
    return false;
  }

  return true;
}
