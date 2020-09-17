const { describe } = require("riteway")
const { eos, encodeName, getBalance, getBalanceFloat, names, getTableRows, isLocal } = require("../scripts/helper")
const { equals } = require("ramda")
const { asset } = require("eosjs/lib/schema")

const { organization, accounts, token, firstuser, seconduser, thirduser, bank, settings, harvest, history } = names

let eosDevKey = "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}  

// describe('organization', async assert => {

//     if (!isLocal()) {
//         console.log("only run unit tests on local - don't reset accounts on mainnet or testnet")
//         return
//     }

//     const contracts = await Promise.all([
//         eos.contract(organization),
//         eos.contract(token),
//         eos.contract(accounts),
//         eos.contract(settings),
//         eos.contract(harvest),
//     ]).then(([organization, token, accounts, settings, harvest]) => ({
//         organization, token, accounts, settings, harvest
//     }))

//     console.log('reset organization')
//     await contracts.organization.reset({ authorization: `${organization}@active` })

//     console.log('reset token stats')
//     await contracts.token.resetweekly({ authorization: `${token}@active` })

//     console.log('accounts reset')
//     await contracts.accounts.reset({ authorization: `${accounts}@active` })
    
//     console.log('harvest reset')
//     await contracts.harvest.reset({ authorization: `${harvest}@active` })

//     console.log('configure')
//     await contracts.settings.configure('planted', 500000, { authorization: `${settings}@active` })

//     console.log('join users')
//     await contracts.accounts.adduser(firstuser, 'first user', 'individual', { authorization: `${accounts}@active` })
//     await contracts.accounts.adduser(seconduser, 'second user', 'individual', { authorization: `${accounts}@active` })

//     console.log('add rep')
//     await contracts.accounts.addrep(firstuser, 10000, { authorization: `${accounts}@active` })
//     await contracts.accounts.addrep(seconduser, 13000, { authorization: `${accounts}@active` })

//     console.log('create balance')
//     await contracts.token.transfer(firstuser, organization, "400.0000 SEEDS", "Initial supply", { authorization: `${firstuser}@active` })
//     await contracts.token.transfer(seconduser, organization, "200.0000 SEEDS", "Initial supply", { authorization: `${seconduser}@active` })

//     const initialBalances = await getTableRows({
//         code: organization,
//         scope: organization,
//         table: 'sponsors',
//         json: true
//     })

//     console.log('create organization')
//     await contracts.organization.create(firstuser, 'testorg1', "Org Number 1", eosDevKey, { authorization: `${firstuser}@active` })
//     await contracts.organization.create(firstuser, 'testorg2', "Org 2", eosDevKey,  { authorization: `${firstuser}@active` })
//     await contracts.organization.create(seconduser, 'testorg3', "Org 3 - Test, Inc.", eosDevKey, { authorization: `${seconduser}@active` })

//     let plantedAfter = (await getTableRows({
//         code: harvest,
//         scope: harvest,
//         table: 'balances',
//         json: true
//     })).rows.map( item => parseInt(item.planted) )

//     const initialOrgs = await getTableRows({
//         code: organization,
//         scope: organization,
//         table: 'organization',
//         json: true
//     })

//     console.log('add member')
//     await contracts.organization.addmember('testorg1', firstuser, seconduser, 'admin', { authorization: `${firstuser}@active` })
//     await contracts.organization.addmember('testorg3', seconduser, firstuser, 'admin', { authorization: `${seconduser}@active` })
    

//     const members1 = await getTableRows({
//         code: organization,
//         scope: 'testorg1',
//         table: 'members',
//         json: true
//     })

//     const members2 = await getTableRows({
//         code: organization,
//         scope: 'testorg3',
//         table: 'members',
//         json: true
//     })

//     console.log('destroy organization')
//     await contracts.organization.destroy('testorg2', firstuser, { authorization: `${firstuser}@active` })
//     //await contracts.organization.refund(firstuser, "50.0000 SEEDS", { authorization: `${firstuser}@active` })

//     const orgs = await getTableRows({
//         code: organization,
//         scope: organization,
//         table: 'organization',
//         json: true
//     })

//     console.log('change owner')

//     try{
//         console.log('remove owner')
//         await contracts.organization.removemember('testorg3', firstuser, firstuser, { authorization: `${firstuser}@active` })
//     }
//     catch(err){
//         console.log('You can not remove de owner')
//     }

//     await contracts.organization.changeowner('testorg3', seconduser, firstuser, { authorization: `${seconduser}@active` })
//     await contracts.organization.changerole('testorg3', firstuser, seconduser, 'testrole', { authorization: `${firstuser}@active` })

//     const currentRoles = await getTableRows({
//         code: organization,
//         scope: organization,
//         table: 'organization',
//         json: true
//     })

//     console.log('remove member')
//     await contracts.organization.removemember('testorg3', firstuser, seconduser, { authorization: `${firstuser}@active` })

//     const members3 = await getTableRows({
//         code: organization,
//         scope: 'testorg3',
//         table: 'members',
//         json: true
//     })

//     console.log('add regen')
//     await contracts.organization.addregen('testorg1', firstuser, 1, { authorization: `${firstuser}@active` })
//     await contracts.organization.subregen('testorg3', seconduser, 1, { authorization: `${seconduser}@active` })
    
//     const regen = await getTableRows({
//         code: organization,
//         scope: organization,
//         table: 'organization',
//         json: true
//     })


//     try{
//         console.log('create organization')
//         await contracts.organization.create(thirduser, 'testorg4', eosDevKey, { authorization: `${thirduser}@active`  })
//     }
//     catch(err){
//         console.log('user thoes not have a balance entry')
//     }

//     try{
//         console.log('create organization')
//         await contracts.token.transfer(thirduser, organization, "20.0000 SEEDS", "Initial supply", { authorization: `${thirduser}@active` })
//         await contracts.organization.create(thirduser, 'testorg4', eosDevKey, { authorization: `${thirduser}@active`  })
//     }
//     catch(err){
//         console.log('user has not enough balance')
//     }

//     assert({
//         given: 'organisations were created',
//         should: 'they have planted scores',
//         actual: plantedAfter,
//         expected: [600, 200, 200, 200] // 600 is orgs contract, the other 3 are 3 created orgs
//     })

//     assert({
//         given: 'firstuser and second user transfer to organization contract',
//         should: 'update the sponsors table',
//         actual: initialBalances.rows.map(row => { return row }),
//         expected: [
//             {
//                 account: 'orgs.seeds',
//                 balance: '600.0000 SEEDS'
//             },
//             {
//                 account: 'seedsuseraaa',
//                 balance: '400.0000 SEEDS'
//             },
//             {
//                 account: 'seedsuserbbb',
//                 balance: '200.0000 SEEDS'
//             },
//         ]
//     })

//     assert({
//         given: 'organizations created',
//         should: 'update the organizations table',
//         actual: initialOrgs.rows.map(row => { return row }),
//         expected: [
//             {
//                 org_name: 'testorg1',
//                 owner: 'seedsuseraaa',
//                 status: 0,
//                 regen: 0,
//                 reputation: 0,
//                 voice: 0,
//                 planted: "200.0000 SEEDS"
//             },
//             {
//                 org_name: 'testorg2',
//                 owner: 'seedsuseraaa',
//                 status: 0,
//                 regen: 0,
//                 reputation: 0,
//                 voice: 0,
//                 planted: "200.0000 SEEDS"
//             },
//             {
//                 org_name: 'testorg3',
//                 owner: 'seedsuserbbb',
//                 status: 0,
//                 regen: 0,
//                 reputation: 0,
//                 voice: 0,
//                 planted: "200.0000 SEEDS"
//             }
//         ]
//     })

//     assert({
//         given: 'Orgs having members',
//         should: 'Change roles properly',
//         actual: members1.rows.map(row => {
//             return row
//         }),
//         expected: [
//             {
//                 account: 'seedsuseraaa',
//                 role: ''
//             },
//             {
//                 account: 'seedsuserbbb',
//                 role: 'admin'
//             }
//         ]
//     })

//     assert({
//         given: 'Orgs having members',
//         should: 'Change roles properly',
//         actual: members2.rows.map(row => {
//             return row
//         }),
//         expected: [
//             {
//                 account: 'seedsuseraaa',
//                 role: 'admin'
//             },
//             {
//                 account: 'seedsuserbbb',
//                 role: ''
//             }
//         ]
//     })

//     assert({
//         given: 'Organization destroyed and the refund function called',
//         should: 'erase the organization its members and give the funds back to the user',
//         actual: orgs.rows.map(row => {
//             return row
//         }),
//         expected: [
//             {
//                 org_name: 'testorg1',
//                 owner: 'seedsuseraaa',
//                 status: 0,
//                 regen: 0,
//                 reputation: 0,
//                 voice: 0,
//                 planted: "200.0000 SEEDS"
//             },
//             {
//                 org_name: 'testorg3',
//                 owner: 'seedsuserbbb',
//                 status: 0,
//                 regen: 0,
//                 reputation: 0,
//                 voice: 0,
//                 planted: "200.0000 SEEDS"
//             }
//         ]
//     })

//     assert({
//         given: 'Roles changed',
//         should: 'Update the organization information',
//         actual: currentRoles.rows.map(row => {return row}),
//         expected: [
//             {
//                 org_name: 'testorg1',
//                 owner: 'seedsuseraaa',
//                 status: 0,
//                 regen: 0,
//                 reputation: 0,
//                 voice: 0,
//                 planted: "200.0000 SEEDS"
//             },
//             {
//                 org_name: 'testorg3',
//                 owner: 'seedsuseraaa',
//                 status: 0,
//                 regen: 0,
//                 reputation: 0,
//                 voice: 0,
//                 planted: "200.0000 SEEDS"
//             }
//         ]
//     })

//     assert({
//         given: 'A memeber removed',
//         should: 'Erase the member',
//         actual: members3.rows.map(row => {
//             return row
//         }),
//         expected: [
//             {
//                 account: 'seedsuseraaa',
//                 role: 'admin'
//             }
//         ]
//     })

//     assert({
//         given: 'Users voted',
//         should: 'Update the regen points according to users\' rep',
//         actual: regen.rows.map(row => {
//             return row
//         }),
//         expected: [
//             {
//                 org_name: 'testorg1',
//                 owner: 'seedsuseraaa',
//                 status: 0,
//                 regen: 10000,
//                 reputation: 0,
//                 voice: 0,
//                 planted: "200.0000 SEEDS"
//             },
//             {
//                 org_name: 'testorg3',
//                 owner: 'seedsuseraaa',
//                 status: 0,
//                 regen: -13000,
//                 reputation: 0,
//                 voice: 0,
//                 planted: "200.0000 SEEDS"
//             }
//         ]
//     })
// })


// describe('app', async assert => {

//     if (!isLocal()) {
//         console.log("only run unit tests on local - don't reset accounts on mainnet or testnet")
//         return
//     }

//     const contracts = await Promise.all([
//         eos.contract(organization),
//         eos.contract(token),
//         eos.contract(accounts),
//         eos.contract(settings),
//         eos.contract(harvest),
//     ]).then(([organization, token, accounts, settings, harvest]) => ({
//         organization, token, accounts, settings, harvest
//     }))

//     console.log('changing batch size')
//     await contracts.settings.configure('batchsize', 1, { authorization: `${settings}@active` })

//     console.log('reset organization')
//     await contracts.organization.reset({ authorization: `${organization}@active` })

//     console.log('accounts reset')
//     await contracts.accounts.reset({ authorization: `${accounts}@active` })

//     console.log('join users')
//     await contracts.accounts.adduser(firstuser, 'first user', 'individual', { authorization: `${accounts}@active` })
//     await contracts.accounts.adduser(seconduser, 'second user', 'individual', { authorization: `${accounts}@active` })
    
//     console.log('create organization')
//     await contracts.token.transfer(firstuser, organization, "400.0000 SEEDS", "Initial supply", { authorization: `${firstuser}@active` })
//     await contracts.token.transfer(seconduser, organization, "400.0000 SEEDS", "Initial supply", { authorization: `${seconduser}@active` })
//     await contracts.organization.create(firstuser, 'testorg1', "Org Number 1", eosDevKey, { authorization: `${firstuser}@active` })
//     await contracts.organization.create(seconduser, 'testorg2', "Org Number 2", eosDevKey, { authorization: `${seconduser}@active` })

//     console.log('register app')
//     await contracts.organization.registerapp(firstuser, 'testorg1', 'app1', 'app long name', { authorization: `${firstuser}@active` })
//     await contracts.organization.registerapp(seconduser, 'testorg2', 'app2', 'app long name 2', { authorization: `${seconduser}@active` })

//     let createOrgNotBeingOwner = true
//     try {
//         await contracts.organization.registerapp(seconduser, 'testorg1', 'app3', 'app3 long name', { authorization: `${seconduser}@active` })
//         console.log('app3 registered (not expected)')
//     } catch (err) {
//         createOrgNotBeingOwner = false
//         console.log('only the owner can register an app (expected)')
//     }

//     console.log('use app')
//     await contracts.organization.appuse('app1', firstuser, { authorization: `${firstuser}@active` })
//     await sleep(300)
    
//     for (let i = 0; i < 10; i++) {
//         await contracts.organization.appuse('app1', seconduser, { authorization: `${seconduser}@active` })
//         await sleep(400)
//     }

//     await contracts.organization.appuse('app2', seconduser, { authorization: `${seconduser}@active` })

//     const daus1Table = await getTableRows({
//         code: organization,
//         scope: 'app1',
//         table: 'daus',
//         json: true
//     })

//     const daus2Table = await getTableRows({
//         code: organization,
//         scope: 'app2',
//         table: 'daus',
//         json: true
//     })

//     const appsTable = await getTableRows({
//         code: organization,
//         scope: organization,
//         table: 'apps',
//         json: true
//     })
//     const apps = appsTable.rows

//     console.log('clean daus today')
//     await contracts.organization.cleandaus({ authorization: `${organization}@active` })
//     await sleep(3000)

//     const daus1TableAfterClean1 = await getTableRows({
//         code: organization,
//         scope: 'app1',
//         table: 'daus',
//         json: true
//     })

//     const daus2TableAfterClean1 = await getTableRows({
//         code: organization,
//         scope: 'app2',
//         table: 'daus',
//         json: true
//     })

//     let today = new Date()
//     today.setUTCHours(0, 0, 0, 0)
//     today = today.getTime() / 1000

//     let tomorrow = new Date()
//     tomorrow.setUTCHours(0, 0, 0, 0)
//     tomorrow.setDate(tomorrow.getDate() + 1)
//     tomorrow = tomorrow.getTime() / 1000

//     console.log('clean app1 dau')
//     await contracts.organization.cleandau('app1', tomorrow, 0, { authorization: `${organization}@active` })
//     await sleep(2000)

//     const daus1TableAfterClean2 = await getTableRows({
//         code: organization,
//         scope: 'app1',
//         table: 'daus',
//         json: true
//     })

//     const daus1History1 = await getTableRows({
//         code: organization,
//         scope: 'app1',
//         table: 'dauhistory',
//         json: true
//     })

//     console.log('ban app')
//     await contracts.organization.banapp('app1', { authorization: `${organization}@active` })

//     const appsTableAfterBan = await getTableRows({
//         code: organization,
//         scope: organization,
//         table: 'apps',
//         json: true
//     })

//     console.log('reset settings')
//     await contracts.settings.reset({ authorization: `${settings}@active` })

//     assert({
//         given: 'registered an app',
//         should: 'have an entry in the apps table',
//         actual: apps,
//         expected: [
//             { 
//                 app_name: 'app1',
//                 org_name: 'testorg1',
//                 app_long_name: 'app long name',
//                 is_banned: 0,
//                 number_of_uses: 11
//             },
//             { 
//                 app_name: 'app2',
//                 org_name: 'testorg2',
//                 app_long_name: 'app long name 2',
//                 is_banned: 0,
//                 number_of_uses: 1
//             }
//         ]
//     })

//     assert({
//         given: 'appuse called',
//         should: 'increment the app use counter',
//         actual: [
//             daus1Table.rows.map(values => values.number_app_uses),
//             daus2Table.rows.map(values => values.number_app_uses)
//         ],
//         expected: [[1, 10], [1]]
//     })

//     assert({
//         given: 'register app by not the owner',
//         should: 'not register the app',
//         actual: createOrgNotBeingOwner,
//         expected: false
//     })

//     assert({
//         given: 'call cleandaus on the same day',
//         should: 'not clean daus app1 table',
//         actual: daus1TableAfterClean1.rows,
//         expected: daus1Table.rows
//     })

//     assert({
//         given: 'call cleandaus on the same day',
//         should: 'not clean daus app2 table',
//         actual: daus2TableAfterClean1.rows,
//         expected: daus2Table.rows
//     })

//     assert({
//         given: 'call cleandau for app1 on a different day',
//         should: 'clean daus app1 table',
//         actual: daus1TableAfterClean2.rows,
//         expected: [ 
//             { account: 'seedsuseraaa', date: tomorrow, number_app_uses: 0 },
//             { account: 'seedsuserbbb', date: tomorrow, number_app_uses: 0 } 
//         ]
//     })

//     assert({
//         given: 'call cleandau for app1 on a different day',
//         should: 'have entries in the dau history table',
//         actual: daus1History1.rows,
//         expected: [ 
//             { 
//                 dau_history_id: 0,
//                 account: 'seedsuseraaa',
//                 date: today,
//                 number_app_uses: 1 
//             },
//             { 
//                 dau_history_id: 1,
//                 account: 'seedsuserbbb',
//                 date: today,
//                 number_app_uses: 10 
//             } 
//         ]
//     })

//     assert({
//         given: 'ban app called',
//         should: 'ban the app',
//         actual: appsTableAfterBan.rows,
//         expected: [
//             { 
//                 app_name: 'app1',
//                 org_name: 'testorg1',
//                 app_long_name: 'app long name',
//                 is_banned: 1,
//                 number_of_uses: 11
//             },
//             { 
//                 app_name: 'app2',
//                 org_name: 'testorg2',
//                 app_long_name: 'app long name 2',
//                 is_banned: 0,
//                 number_of_uses: 1
//             }
//         ]
//     })

// })


describe('organization scores + status', async assert => {
    if (!isLocal()) {
        console.log("only run unit tests on local - don't reset accounts on mainnet or testnet")
        return
    }

    const contracts = await Promise.all([
        eos.contract(organization),
        eos.contract(token),
        eos.contract(accounts),
        eos.contract(settings),
        eos.contract(harvest),
        eos.contract(history)
    ]).then(([organization, token, accounts, settings, harvest, history]) => ({
        organization, token, accounts, settings, harvest, history
    }))

    console.log('reset organization')
    await contracts.organization.reset({ authorization: `${organization}@active` })

    console.log('reset token stats')
    await contracts.token.resetweekly({ authorization: `${token}@active` })

    console.log('accounts reset')
    await contracts.accounts.reset({ authorization: `${accounts}@active` })
    
    console.log('harvest reset')
    await contracts.harvest.reset({ authorization: `${harvest}@active` })

    console.log('configure')
    await contracts.settings.reset({ authorization: `${settings}@active` })

    console.log('join users')
    await contracts.accounts.adduser(firstuser, 'first user', 'individual', { authorization: `${accounts}@active` })
    await contracts.accounts.adduser(seconduser, 'second user', 'individual', { authorization: `${accounts}@active` })
    await contracts.accounts.adduser(thirduser, 'third user', 'individual', { authorization: `${accounts}@active` })

    console.log('add rep')
    await contracts.accounts.addrep(firstuser, 20000, { authorization: `${accounts}@active` })
    await contracts.accounts.addrep(seconduser, 13000, { authorization: `${accounts}@active` })
    await contracts.accounts.addrep(thirduser, 1000, { authorization: `${accounts}@active` })

    console.log('create balance')
    await contracts.token.transfer(firstuser, organization, "400.0000 SEEDS", "Initial supply", { authorization: `${firstuser}@active` })
    await contracts.token.transfer(seconduser, organization, "600.0000 SEEDS", "Initial supply", { authorization: `${seconduser}@active` })

    const initialBalances = await getTableRows({
        code: organization,
        scope: organization,
        table: 'sponsors',
        json: true
    })

    const org1 = 'testorg1'
    const org2 = 'testorg2'
    const org3 = 'testorg3'
    const org4 = 'testorg4'
    const org5 = 'testorg5'

    console.log('create organizations')
    await contracts.organization.create(firstuser, org1, "Org Number 1", eosDevKey, { authorization: `${firstuser}@active` })
    await contracts.organization.create(firstuser, org2, "Org 2", eosDevKey,  { authorization: `${firstuser}@active` })
    await contracts.organization.create(seconduser, org3, "Org 3 - Test, Inc.", eosDevKey, { authorization: `${seconduser}@active` })
    await contracts.organization.create(seconduser, org4, "Org 4 - Test, Inc.", eosDevKey, { authorization: `${seconduser}@active` })
    await contracts.organization.create(seconduser, org5, "Org 5 - Test, Inc.", eosDevKey, { authorization: `${seconduser}@active` })

    console.log('modify regen')
    await contracts.organization.subregen(org1, firstuser, 2, { authorization: `${firstuser}@active` })
    await contracts.organization.subregen(org1, seconduser, 3, { authorization: `${seconduser}@active` })

    await contracts.organization.addregen(org2, firstuser, 4, { authorization: `${firstuser}@active` })
    await contracts.organization.addregen(org2, seconduser, 4, { authorization: `${seconduser}@active` })

    await contracts.organization.addregen(org3, firstuser, 1, { authorization: `${firstuser}@active` })
    await contracts.organization.subregen(org3, seconduser, 1, { authorization: `${seconduser}@active` })
    await contracts.organization.subregen(org3, thirduser, 1, { authorization: `${thirduser}@active` })

    await contracts.organization.addregen(org4, firstuser, 2, { authorization: `${firstuser}@active` })
    await contracts.organization.addregen(org4, seconduser, 2, { authorization: `${seconduser}@active` })

    await contracts.organization.addregen(org5, firstuser, 7, { authorization: `${firstuser}@active` })
    await contracts.organization.addregen(org5, seconduser, 7, { authorization: `${seconduser}@active` })
    
    await contracts.organization.calcmregens({ authorization: `${organization}@active` })
    await sleep(200)

    await contracts.accounts.testsetcbs(org1, 100, { authorization: `${accounts}@active` })
    await contracts.accounts.testsetcbs(org2, 75, { authorization: `${accounts}@active` })
    await contracts.accounts.testsetcbs(org3, 50, { authorization: `${accounts}@active` })
    await contracts.accounts.testsetcbs(org4, 25, { authorization: `${accounts}@active` })
    await contracts.accounts.testsetcbs(org5, 1, { authorization: `${accounts}@active` })

    await contracts.accounts.rankreps({ authorization: `${accounts}@active` })
    await sleep(200)

    await contracts.history.reset(org1, { authorization: `${history}@active` })
    await contracts.history.reset(org2, { authorization: `${history}@active` })
    await contracts.history.reset(org3, { authorization: `${history}@active` })
    await contracts.history.reset(org4, { authorization: `${history}@active` })
    await contracts.history.reset(org5, { authorization: `${history}@active` })

    console.log('transfer volume of seeds')
    await contracts.token.transfer(firstuser, org1, "150.0000 SEEDS", "Test supply", { authorization: `${firstuser}@active` })
    await contracts.token.transfer(org1, firstuser, "50.0000 SEEDS", "Test supply", { authorization: `${org1}@active` })
    await contracts.token.transfer(org1, seconduser, "50.0000 SEEDS", "Test supply", { authorization: `${org1}@active` })
    await contracts.token.transfer(org1, thirduser, "50.0000 SEEDS", "Test supply", { authorization: `${org1}@active` })

    await contracts.token.transfer(firstuser, org2, "300.0000 SEEDS", "Test supply", { authorization: `${firstuser}@active` })
    await contracts.token.transfer(org2, firstuser, "100.0000 SEEDS", "Test supply", { authorization: `${org2}@active` })
    await contracts.token.transfer(org2, seconduser, "100.0000 SEEDS", "Test supply", { authorization: `${org2}@active` })
    await contracts.token.transfer(org2, thirduser, "100.0000 SEEDS", "Test supply", { authorization: `${org2}@active` })

    await contracts.token.transfer(firstuser, org3, "600.0000 SEEDS", "Test supply", { authorization: `${firstuser}@active` })
    await contracts.token.transfer(org3, firstuser, "200.0000 SEEDS", "Test supply", { authorization: `${org3}@active` })
    await contracts.token.transfer(org3, seconduser, "200.0000 SEEDS", "Test supply", { authorization: `${org3}@active` })
    await contracts.token.transfer(org3, thirduser, "200.0000 SEEDS", "Test supply", { authorization: `${org3}@active` })

    await contracts.token.transfer(firstuser, org4, "800.0000 SEEDS", "Test supply", { authorization: `${firstuser}@active` })
    await contracts.token.transfer(org4, firstuser, "200.0000 SEEDS", "Test supply", { authorization: `${org4}@active` })
    await contracts.token.transfer(org4, seconduser, "300.0000 SEEDS", "Test supply", { authorization: `${org4}@active` })
    await contracts.token.transfer(org4, thirduser, "300.0000 SEEDS", "Test supply", { authorization: `${org4}@active` })

    await contracts.token.transfer(seconduser, org5, "1000.0000 SEEDS", "Test supply", { authorization: `${seconduser}@active` })
    await contracts.token.transfer(org5, firstuser, "600.0000 SEEDS", "Test supply", { authorization: `${org5}@active` })
    await contracts.token.transfer(org5, seconduser, "200.0000 SEEDS", "Test supply", { authorization: `${org5}@active` })
    await contracts.token.transfer(org5, thirduser, "200.0000 SEEDS", "Test supply", { authorization: `${org5}@active` })    
    
    await sleep(200)

    await contracts.organization.calctrxpts({ authorization: `${organization}@active` })
    await sleep(200)

    await contracts.organization.rankregens({ authorization: `${organization}@active` })
    await contracts.organization.rankcbsorgs({ authorization: `${organization}@active` })
    await contracts.organization.ranktxs({ authorization: `${organization}@active` })

    await sleep(400)

    const regenScores = await getTableRows({
        code: organization,
        scope: organization,
        table: 'regenscores',
        json: true
    })

    const cbsRanks = await getTableRows({
        code: organization,
        scope: organization,
        table: 'cbsorgs',
        json: true
    })

    const txRanks = await getTableRows({
        code: organization,
        scope: organization,
        table: 'txorgpoints',
        json: true
    })

    await contracts.token.transfer(seconduser, org5, "200.0000 SEEDS", "Regen supply", { authorization: `${seconduser}@active` })
    await contracts.token.transfer(org5, harvest, "200.0000 SEEDS", `sow ${org5}`, { authorization: `${org5}@active` })    

    const harvestBalance = await getTableRows({
        code: harvest,
        scope: harvest,
        table: 'balances',
        json: true
    })
    console.log(harvestBalance)
    
    assert({
        given: 'regen scores calculated',
        should: 'rank the orgs properly',
        actual: regenScores.rows,
        expected: [
            { org_name: org2, regen_median: 66000, rank: 50 },
            { org_name: org3, regen_median: -1000, rank: 0 },
            { org_name: org4, regen_median: 33000, rank: 25 },
            { org_name: org5, regen_median: 115500, rank: 75 }
        ]
    })

    assert({
        given: 'cbs calculated',
        should: 'rank the orgs properly',
        actual: cbsRanks.rows,
        expected: [
            { org_name: org1, community_building_score: 100, rank: 80 },
            { org_name: org2, community_building_score: 75, rank: 60 },
            { org_name: org3, community_building_score: 50, rank: 40 },
            { org_name: org4, community_building_score: 25, rank: 20 },
            { org_name: org5, community_building_score: 1, rank: 0 }
        ]
    })

    assert({
        given: 'txs calculated',
        should: 'rank thr orgs properly',
        actual: txRanks.rows,
        expected: [
            { org_name: org1, points: 100, rank: 0 },
            { org_name: org2, points: 200, rank: 20 },
            { org_name: org3, points: 400, rank: 40 },
            { org_name: org4, points: 467, rank: 60 },
            { org_name: org5, points: 934, rank: 80 }
        ]
    })

})


