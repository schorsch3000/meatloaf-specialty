// IPFS - Interplanetary File System
// https://ipfs.tech/
// https://github.com/exmgr/ipfs-client-esp32
// https://developers.cloudflare.com/web3/
// https://ipfs.github.io/public-gateway-checker/
// https://web3.storage/
// https://docs.ipfs.tech/reference/kubo/rpc/#getting-started
// 
// List Directory
// https://ipfs.io/api/v0/ls?arg=Qmbj4vDDkq4kapyfJk27dxzPjQjxvPSW2BhL1GVZngJthq
// https://dweb.link/api/v0/ls?arg=Qmbj4vDDkq4kapyfJk27dxzPjQjxvPSW2BhL1GVZngJthq
//
// List Directory from root CID
// https://dweb.link/api/v0/ls?arg=Qmbj4vDDkq4kapyfJk27dxzPjQjxvPSW2BhL1GVZngJthq/0-9
//
// Download File Directly
// https://ipfs.io/ipfs/QmYuvTLSEmreSvz13zoFEofD7zWNoDBSbESkEZkdovEDTG
// https://dweb.link/ipfs/QmYuvTLSEmreSvz13zoFEofD7zWNoDBSbESkEZkdovEDTG?filename=1000+miler.d64
// https://dweb.link/api/v0/get?arg=QmYuvTLSEmreSvz13zoFEofD7zWNoDBSbESkEZkdovEDTG
//
// Download File from root CID with path
// https://dweb.link/ipfs/Qmbj4vDDkq4kapyfJk27dxzPjQjxvPSW2BhL1GVZngJthq/0-9/1000%20miler.d64
//
// Download File with byte ranges
// read first 20 bytes of a file
// https://dweb.link/api/v0/cat?arg=QmTSGxVkFCshrgCsuZhrmk2ucuRXToEvsRXxM9vwvJWiMp&offset=0&length=20
//
// File Stat
// https://dweb.link/api/v0/object/stat?arg=Qmbj4vDDkq4kapyfJk27dxzPjQjxvPSW2BhL1GVZngJthq/0-9/1000%20miler.d64
// http://localhost:8080/api/v0/object/stat?arg=Qmbj4vDDkq4kapyfJk27dxzPjQjxvPSW2BhL1GVZngJthq/0-9/1000%20miler.d64
// NumLinks = 0, it is a file
// DataSize = {file_size} + 10 bytes
//
// https://github.com/ipfs/kubo/issues/8528
//
// HTTP HEAD to determine DIR or FILE
// content-type: text/html
// etag: "DIRIndex-*" = Directory
// https://ipfs.io/ipfs/Qmbj4vDDkq4kapyfJk27dxzPjQjxvPSW2BhL1GVZngJthq/0-9
//
// content-type: application/octet-stream = File
// content-length: >0  = File
// https://ipfs.io/ipfs/Qmbj4vDDkq4kapyfJk27dxzPjQjxvPSW2BhL1GVZngJthq/0-9/1000 miler.d64
//
