package chipyard

import chisel3._
import chisel3.util.log2Ceil
import chisel3.util.Cat 

import freechips.rocketchip.config.{Config}
import freechips.rocketchip.config._
import freechips.rocketchip.devices.debug._
import freechips.rocketchip.devices.tilelink._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.rocket._
import freechips.rocketchip.tile._
import freechips.rocketchip.tilelink._
import freechips.rocketchip.util._



//============================================================================================================//

//============================================================================================================//
  // length of the key in 32-bit words: 4 words for AES-128, 6 words for AES-192, and 8 words for AES-256
  // rounds: 128 bits key => 10 Rounds, 192 bits key => 12 Rounds, 256 bits key => 14 Rounds
  // round keys needed: 11 round keys for AES-128, 13 keys for AES-192, and 15 keys for AES-256
  
  // Nk=4: Nr=10, 44*32 (11*128)
  // Nk=6, Nr=12, 52*32 (13*128)
  // Nk=8: Nr=14, 60*32 (15*128)
  
case class AES_Config(
  KeySize : Int,
  KeyLength : Int,
  BlockSize : Int,
  NumberRounds : Int,
  KeyBSize : Int,
)

class AES_128 extends AES_Config (
  KeySize = 128,
  KeyLength = 4,
  BlockSize = 4,
  NumberRounds = 10,
  KeyBSize = 44,
)

class AES_192 extends AES_Config (
  KeySize = 192,
  KeyLength = 6,
  BlockSize = 4,
  NumberRounds = 12,
  KeyBSize = 52,
)

class AES_256 extends AES_Config (
  KeySize = 256,
  KeyLength = 8,
  BlockSize = 4,
  NumberRounds = 14,
  KeyBSize = 60,
)

object Sub {
  val ByteTable: Array[Int] = Array(
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
  )

  val InvByteTable: Array[Int] = Array(
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
  )
}
object Sbox {
  val SubByteTable = Wire(Vec(256, UInt(8.W)))
  SubByteTable := VecInit(Sub.ByteTable.map(_&0xff).map(_.U(8.W)))
  def apply(x: UInt):UInt = SubByteTable(x)
}


object RTable {
  val Constants: Array[Int] = Array(0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36)
  }

object Rcon {
  val RconTable = Wire(Vec(10, UInt(8.W)))
  RconTable := VecInit(RTable.Constants.map(_&0xff).map(_.U(8.W)))
  def apply(x: UInt):UInt = RconTable(x)
}


class KeyExpansion(c: AES_Config)(implicit p: Parameters) extends Module{ 
  val io = IO(new Bundle {
  val in = Input(Vec(8, UInt(32.W)))
  val out = Output(Vec(c.KeyBSize, UInt(32.W)))
  })
  val Zeros = RegInit(0.U(24.W))
  def SubWord (w:UInt): UInt = Cat(Sbox(w(31,24)), Sbox(w(23,16)), Sbox(w(15,8)), Sbox(w(7,0)))
  def SubRotateWord (w:UInt): UInt = Cat(Sbox(w(23,16)), Sbox(w(15,8)), Sbox(w(7,0)), Sbox(w(31,24)))
  def RconWord (y:UInt): UInt = Cat(Rcon(y), Zeros)

  for (i <- 0 until c.KeyLength) {
    io.out(i) := io.in(i)
  }
  for (i <- c.KeyLength until c.KeyBSize) {
    if((i % c.KeyLength) == 0) {
    io.out(i) := io.out(i-c.KeyLength) ^SubRotateWord(io.out(i-1)) ^ RconWord(((i/c.KeyLength)-1).asUInt)
    }
    //PARA JÀ DEI SKIP PARA KEY256 BITS (SUBS mod 4)
    else if( (c.KeyLength > 6) && ((i % c.KeyLength) == 4) ){
      io.out(i) := io.out(i-c.KeyLength) ^ SubWord(io.out(i-1))
    }
    else {
      io.out(i) := io.out(i-c.KeyLength) ^ io.out(i-1)
    }
  }      
}


class Cipher_Module(c: AES_Config)(implicit p: Parameters) extends Module{ //32bit decoder implementation without shifts. Compiler doesn't accept shifts of more than 20bits
  

  //=====================INPUTS OUTPUT========================//
  val io = IO(new Bundle {
  val message = Input(Vec(c.BlockSize, UInt(32.W)))
  val key = Input(Vec(c.KeyBSize, UInt(32.W)))
  val en = Input(Bool())
  val out = Output(Vec(c.BlockSize, UInt(32.W)))
  })
  //==============================================================///

  val Sbox1 = Wire(Vec(256, UInt(8.W)))
  Sbox1 := VecInit(Sub.ByteTable.map(_&0xff).map(_.U(8.W)))
  val Sbox2 = Wire(Vec(256, UInt(8.W)))
  Sbox2 := VecInit(Sub.InvByteTable.map(_&0xff).map(_.U(8.W)))

  //==============================================================///

  def SubShiftRow (r0:UInt, r1:UInt, r2:UInt, r3:UInt): UInt = Cat(Sbox1(r0(31,24)), Sbox1(r1(23,16)), Sbox1(r2(15,8)), Sbox1(r3(7,0)))
  def InvSubShiftRow (r0:UInt, r1:UInt, r2:UInt, r3:UInt): UInt = Cat(Sbox2(r0(31,24)), Sbox2(r1(23,16)), Sbox2(r2(15,8)), Sbox2(r3(7,0)))

  //=====================REGISTERS========================//
  
  val message = RegInit(VecInit(Seq.fill(c.BlockSize)(0.U(32.W))))
  val key = RegInit(VecInit(Seq.fill(60)(0.U(32.W))))

  //=====================WIRES========================//
  val state = Wire(Vec(4,UInt(32.W)))  

  for (i <- 0 until c.BlockSize) {
    message(i) := io.message(i)
  }
  when(io.en){
    for (i <- 0 until c.KeyBSize) {
        key(i) := io.key(i)
    }
  }.otherwise{
    for (i <- 0 to c.NumberRounds) {
        for (j <- 0 until 4) {
            key((i*4)+j) := io.key(((c.NumberRounds - i)*4) + j)
        }
    }
  }



  //=====================MODULES========================//
  val round1 = Module(new ROUND_Module())  
  val round2 = Module(new ROUND_Module())  
  val round3 = Module(new ROUND_Module())  
  val round4 = Module(new ROUND_Module())  
  val round5 = Module(new ROUND_Module())  
  val round6 = Module(new ROUND_Module())  
  val round7 = Module(new ROUND_Module())  
  val round8 = Module(new ROUND_Module())  
  val round9 = Module(new ROUND_Module())  
  val round10 = Module(new ROUND_Module())  
  val round11 = Module(new ROUND_Module())  
  val round12 = Module(new ROUND_Module())  
  val round13 = Module(new ROUND_Module())  
  //===================================================//
      round1.io.en := io.en
      round2.io.en := io.en
      round3.io.en := io.en
      round4.io.en := io.en
      round5.io.en := io.en
      round6.io.en := io.en
      round7.io.en := io.en
      round8.io.en := io.en
      round9.io.en := io.en
      round10.io.en := io.en
      round11.io.en := io.en
      round12.io.en := io.en
      round13.io.en := io.en

      for (i <- 0 until c.BlockSize) {
        state(i) := message(i) ^ key(i)
        round1.io.in(i) := state(i)
        round2.io.in(i) := round1.io.out(i)
        round3.io.in(i) := round2.io.out(i)
        round4.io.in(i) := round3.io.out(i)
        round5.io.in(i) := round4.io.out(i)
        round6.io.in(i) := round5.io.out(i)
        round7.io.in(i) := round6.io.out(i)
        round8.io.in(i) := round7.io.out(i)
        round9.io.in(i) := round8.io.out(i)
        round10.io.in(i) := round9.io.out(i)
        round11.io.in(i) := round10.io.out(i)
        round12.io.in(i) := round11.io.out(i)
        round13.io.in(i) := round12.io.out(i)
        round1.io.key(i) := key(i+4)
        round2.io.key(i) := key(i + 8)
        round3.io.key(i) := key(i + 12)
        round4.io.key(i) := key(i + 16)
        round5.io.key(i) := key(i + 20)
        round6.io.key(i) := key(i + 24)
        round7.io.key(i) := key(i + 28)
        round8.io.key(i) := key(i + 32)
        round9.io.key(i) := key(i + 36)
        round10.io.key(i) := key(i + 40)
        round11.io.key(i) := key(i + 44)
        round12.io.key(i) := key(i + 48)
        round13.io.key(i) := key(i + 52)
      }
      when(io.en){
        if(c.KeySize == 128){
            io.out(0) := SubShiftRow(round9.io.out(0), round9.io.out(1), round9.io.out(2), round9.io.out(3)) ^ key(40)
            io.out(1) := SubShiftRow(round9.io.out(1), round9.io.out(2), round9.io.out(3), round9.io.out(0)) ^ key(41)
            io.out(2) := SubShiftRow(round9.io.out(2), round9.io.out(3), round9.io.out(0), round9.io.out(1)) ^ key(42)
            io.out(3) := SubShiftRow(round9.io.out(3), round9.io.out(0), round9.io.out(1), round9.io.out(2)) ^ key(43)
        }
        if(c.KeySize == 192){
            io.out(0) := SubShiftRow(round11.io.out(0), round11.io.out(1), round11.io.out(2), round11.io.out(3)) ^ key(48)
            io.out(1) := SubShiftRow(round11.io.out(1), round11.io.out(2), round11.io.out(3), round11.io.out(0)) ^ key(49)
            io.out(2) := SubShiftRow(round11.io.out(2), round11.io.out(3), round11.io.out(0), round11.io.out(1)) ^ key(40)
            io.out(3) := SubShiftRow(round11.io.out(3), round11.io.out(0), round11.io.out(1), round11.io.out(2)) ^ key(51)
        }
        if(c.KeySize == 256){
            io.out(0) := SubShiftRow(round13.io.out(0), round13.io.out(1), round13.io.out(2), round13.io.out(3)) ^ key(56)
            io.out(1) := SubShiftRow(round13.io.out(1), round13.io.out(2), round13.io.out(3), round13.io.out(0)) ^ key(57)
            io.out(2) := SubShiftRow(round13.io.out(2), round13.io.out(3), round13.io.out(0), round13.io.out(1)) ^ key(58)
            io.out(3) := SubShiftRow(round13.io.out(3), round13.io.out(0), round13.io.out(1), round13.io.out(2)) ^ key(59)
        }
      }.otherwise{
        if(c.KeySize == 128){
            io.out(0) := InvSubShiftRow(round9.io.out(0), round9.io.out(3), round9.io.out(2), round9.io.out(1)) ^ key(40)
            io.out(1) := InvSubShiftRow(round9.io.out(1), round9.io.out(0), round9.io.out(3), round9.io.out(2)) ^ key(41)
            io.out(2) := InvSubShiftRow(round9.io.out(2), round9.io.out(1), round9.io.out(0), round9.io.out(3)) ^ key(42)
            io.out(3) := InvSubShiftRow(round9.io.out(3), round9.io.out(2), round9.io.out(1), round9.io.out(0)) ^ key(43)
        }
        if(c.KeySize == 192){
            io.out(0) := InvSubShiftRow(round11.io.out(0), round11.io.out(3), round11.io.out(2), round11.io.out(1)) ^ key(48)
            io.out(1) := InvSubShiftRow(round11.io.out(1), round11.io.out(0), round11.io.out(3), round11.io.out(2)) ^ key(49)
            io.out(2) := InvSubShiftRow(round11.io.out(2), round11.io.out(1), round11.io.out(0), round11.io.out(3)) ^ key(40)
            io.out(3) := InvSubShiftRow(round11.io.out(3), round11.io.out(2), round11.io.out(1), round11.io.out(0)) ^ key(51)
        }
        if(c.KeySize == 256){
            io.out(0) := InvSubShiftRow(round13.io.out(0), round13.io.out(3), round13.io.out(2), round13.io.out(1)) ^ key(56)
            io.out(1) := InvSubShiftRow(round13.io.out(1), round13.io.out(0), round13.io.out(3), round13.io.out(2)) ^ key(57)
            io.out(2) := InvSubShiftRow(round13.io.out(2), round13.io.out(1), round13.io.out(0), round13.io.out(3)) ^ key(58)
            io.out(3) := InvSubShiftRow(round13.io.out(3), round13.io.out(2), round13.io.out(1), round13.io.out(0)) ^ key(59)
        }
      } 
  
//=================================================================================================================///

}

class ROUND_Module()(implicit p: Parameters) extends Module{ //32bit decoder implementation without shifts. Compiler doesn't accept shifts of more than 20bits
  val io = IO(new Bundle {
  val in = Input(Vec(4, UInt(32.W)))
  val key = Input(Vec(4, UInt(32.W)))
  val en = Input(Bool())
  val out = Output(Vec(4, UInt(32.W)))
  })

//=====================WIRES========================//
  val temp = Wire(Vec(4,UInt(32.W)))  
  val test_round =  RegInit(0.U(32.W))

  val Sbox1 = Wire(Vec(256, UInt(8.W)))
  Sbox1 := VecInit(Sub.ByteTable.map(_&0xff).map(_.U(8.W)))

  val Sbox2 = Wire(Vec(256, UInt(8.W)))
  Sbox2 := VecInit(Sub.InvByteTable.map(_&0xff).map(_.U(8.W)))
//========================================================///

  //==================================================FUCTIONS DEFINES==================================================//  
  def SubShiftRow (r0:UInt, r1:UInt, r2:UInt, r3:UInt): UInt = Cat(Sbox1(r0(31,24)), Sbox1(r1(23,16)), Sbox1(r2(15,8)), Sbox1(r3(7,0)))
  def InvSubShiftRow (r0:UInt, r1:UInt, r2:UInt, r3:UInt): UInt = Cat(Sbox2(r0(31,24)), Sbox2(r1(23,16)), Sbox2(r2(15,8)), Sbox2(r3(7,0)))
 
  def x02(x:UInt):UInt = Mux(x(7), Cat(x(6,0), 0.U(1.W)) ^ 0x1b.U(8.W), Cat(x(6,0), 0.U(1.W)))
  def x03(x:UInt):UInt = x02(x) ^ x
  def x09(x:UInt):UInt = x02(x02(x02(x))) ^ x
  def x0b(x:UInt):UInt = x02(x02(x02(x))) ^ x02(x) ^ x
  def x0d(x:UInt):UInt = x02(x02(x02(x))) ^ x02(x02(x)) ^ x
  def x0e(x:UInt):UInt = x02(x02(x02(x))) ^ x02(x02(x)) ^ x02(x)

  def En_Line0(w:UInt):UInt = x02(w(31,24)) ^ x03(w(23,16)) ^ w(15,8)      ^ w(7,0)
  def En_Line1(w:UInt):UInt = w(31,24)      ^ x02(w(23,16)) ^ x03(w(15,8)) ^ w(7,0)
  def En_Line2(w:UInt):UInt = w(31,24)      ^ w(23,16)      ^ x02(w(15,8)) ^ x03(w(7,0))
  def En_Line3(w:UInt):UInt = x03(w(31,24)) ^ w(23,16)      ^ w(15,8)      ^ x02(w(7,0))

  def De_Line0(w:UInt):UInt = x0e(w(31,24)) ^ x0b(w(23,16)) ^ x0d(w(15,8)) ^ x09(w(7,0))
  def De_Line1(w:UInt):UInt = x09(w(31,24)) ^ x0e(w(23,16)) ^ x0b(w(15,8)) ^ x0d(w(7,0))
  def De_Line2(w:UInt):UInt = x0d(w(31,24)) ^ x09(w(23,16)) ^ x0e(w(15,8)) ^ x0b(w(7,0))
  def De_Line3(w:UInt):UInt = x0b(w(31,24)) ^ x0d(w(23,16)) ^ x09(w(15,8)) ^ x0e(w(7,0))

  //======================================================================================================================//
  when(io.en){
      temp(0) := SubShiftRow(io.in(0), io.in(1), io.in(2), io.in(3))
      temp(1) := SubShiftRow(io.in(1), io.in(2), io.in(3), io.in(0))
      temp(2) := SubShiftRow(io.in(2), io.in(3), io.in(0), io.in(1))
      temp(3) := SubShiftRow(io.in(3), io.in(0), io.in(1), io.in(2))
      io.out(0) := Cat(En_Line0(temp(0)), En_Line1(temp(0)), En_Line2(temp(0)), En_Line3(temp(0))) ^ io.key(0)
      io.out(1) := Cat(En_Line0(temp(1)), En_Line1(temp(1)), En_Line2(temp(1)), En_Line3(temp(1))) ^ io.key(1)
      io.out(2) := Cat(En_Line0(temp(2)), En_Line1(temp(2)), En_Line2(temp(2)), En_Line3(temp(2))) ^ io.key(2)
      io.out(3) := Cat(En_Line0(temp(3)), En_Line1(temp(3)), En_Line2(temp(3)), En_Line3(temp(3))) ^ io.key(3)
    }.otherwise{
      temp(0) := InvSubShiftRow(io.in(0), io.in(3), io.in(2), io.in(1)) ^ io.key(0)
      temp(1) := InvSubShiftRow(io.in(1), io.in(0), io.in(3), io.in(2)) ^ io.key(1)
      temp(2) := InvSubShiftRow(io.in(2), io.in(1), io.in(0), io.in(3)) ^ io.key(2)
      temp(3) := InvSubShiftRow(io.in(3), io.in(2), io.in(1), io.in(0)) ^ io.key(3) 
      io.out(0) := Cat(De_Line0(temp(0)), De_Line1(temp(0)), De_Line2(temp(0)), De_Line3(temp(0))) 
      io.out(1) := Cat(De_Line0(temp(1)), De_Line1(temp(1)), De_Line2(temp(1)), De_Line3(temp(1))) 
      io.out(2) := Cat(De_Line0(temp(2)), De_Line1(temp(2)), De_Line2(temp(2)), De_Line3(temp(2))) 
      io.out(3) := Cat(De_Line0(temp(3)), De_Line1(temp(3)), De_Line2(temp(3)), De_Line3(temp(3))) 
    }
}




class KeyExpansionExample(opcodes: OpcodeSet, c: AES_Config = new AES_128)(implicit p: Parameters) extends LazyRoCC(opcodes) {
  override lazy val module = new KeyExpansionExampleModuleImp(this, c)
}

class KeyExpansionExampleModuleImp(outer: KeyExpansionExample, c: AES_Config)(implicit p: Parameters) extends LazyRoCCModuleImp(outer)
    with HasCoreParameters {
  

  // ------------> Alias <-------------
  val inputData1 = io.cmd.bits.rs1
  val inputData2 = io.cmd.bits.rs2
  val funct = io.cmd.bits.inst.funct
  // ----------------------------------

  val ReceiveKey_0 = funct === 0.U
  val ReceiveKey_1 = funct === 1.U
  val ReceiveKey_2 = funct === 2.U
  val ReceiveKey_3 = funct === 3.U
  val ReadKey_4 = funct === 4.U
  val ReadKey_5 = funct === 5.U
  val ReceiveInput_6 = funct === 6.U
  val ReceiveInput_7 = funct === 7.U
  val ReadInput_8 = funct === 8.U
  val ReadOut_9 = funct === 9.U
  val Set_en_10 = funct === 10.U
  val DSet_en_11 = funct === 11.U
  val ReadOut_12 = funct === 12.U
  val ReadOut_13 = funct === 13.U
    
//=====================AES VALUES==================


  val Keys = Module(new KeyExpansion(c))  
  val Cipher = Module(new Cipher_Module(c))

  val data = Wire(UInt(32.W))   
  data := DontCare
  val key =  RegInit(VecInit(Seq.fill(8)(0.U(32.W))))
  val plaintext =  RegInit(VecInit(Seq.fill(4)(0.U(32.W))))  
  val en =  Reg(Bool())

  for (i <- 0 until 8) {
     Keys.io.in(i) := key(i) 
  }
  for (i <- 0 until c.KeyBSize) {
    Cipher.io.key(i) := Keys.io.out(i)
  }
  for (i <- 0 until 4) {
    Cipher.io.message(i) := plaintext(i)
  }
  Cipher.io.en := en
//=====================AES VALUES==================


  when (io.cmd.fire()){
    when(ReceiveKey_0) {
      key(0) := inputData1
      key(1) := inputData2
    }
     when(ReceiveKey_1) {
      key(2) := inputData1
      key(3) := inputData2 
    }
     when(ReceiveKey_2) {
      key(4) := inputData1
      key(5) := inputData2
    }
     when(ReceiveKey_3) {
      key(6) := inputData1
      key(7) := inputData2
    }
    when(ReadKey_4) {
      data := key(inputData1)
    }
    when(ReadKey_5) {
      data := Keys.io.out(inputData1)
    }    
    when(ReceiveInput_6) {
      plaintext(0) := inputData1
      plaintext(1) := inputData2
    }
     when(ReceiveInput_7) {
      plaintext(2) := inputData1
      plaintext(3) := inputData2
    }
    when(ReadInput_8) {
      data := plaintext(inputData1)
    }
    when(ReadOut_9) {
      data := Cipher.io.out(inputData1)
    }
    when(Set_en_10) {
      en := true.B
    }
    when(DSet_en_11) {
      en := false.B
    }
  }

 
  io.cmd.ready := !(io.cmd.bits.inst.xd && !io.resp.ready)
  io.resp.valid := io.cmd.valid && io.cmd.bits.inst.xd 
  io.resp.bits.rd := io.cmd.bits.inst.rd
  io.resp.bits.data := data
  io.busy := io.cmd.valid
  io.interrupt := false.B
}
