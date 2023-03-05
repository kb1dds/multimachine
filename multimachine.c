/* A nondeterministic machine architecture with random execution order
 * 
 * Copyright (c) 2023, Michael Robinson
 * Available under MIT license 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Size of simulated memory in bytes */
#define MEMSIZ 1024

/* Allow absolute addresses? */
#define ABSFLG 0

/* Decode opcodes? */
#define OPCODES 0

/* Instructions for simulated machine consist of four words: 
 * 0. An opcode,
 * 1. Source operand 1,
 * 2. Source operand 2,
 * 3. Destination operand
 *
 * Indirection of the operands are determined by flags within the opcode 
 */

/*** Prototypes ***/

/* Validate an address within the memory buffer */
char *buffer_address( unsigned char *membuf, unsigned length, unsigned char *address );

/* Load byte from memory buffer */
char buffer_access( unsigned char *membuf, unsigned length, unsigned char *address );

/* Execute an instruction located at entpt, updating memory buffer as appropriate  */
void run_instruction( unsigned char *membuf, unsigned length, unsigned char *entrypt );

/* Print contents of memory buffer */
void print_membuf( unsigned char *membuf, unsigned length );

/*** Definitions ***/

char *buffer_address( unsigned char *membuf, unsigned length, unsigned char *address ){
  unsigned char *valid_address;

  /* Validate address */
  valid_address = address;

  while(valid_address < membuf)
    valid_address += length;
  while(valid_address >= (membuf+length))
    valid_address -= length;

  return valid_address;
}

char buffer_access( unsigned char *membuf, unsigned length, unsigned char *address ){
  unsigned char *valid_address;

  valid_address=buffer_address( membuf, length, address );
  return *valid_address;
}

void run_instruction( unsigned char *membuf, unsigned length, unsigned char *entrypt ){
  unsigned char opcode, op1, op2, dest;
  unsigned char stored_address;
  unsigned char *address;

  opcode=buffer_access(membuf,length,entrypt);

  /* Load first operand */
  if( ABSFLG && (opcode & 0x80) ){ /* Absolute source */
    stored_address=buffer_access(membuf,length,entrypt+1);
    address=membuf + stored_address;
    op1=buffer_access(membuf,length,address);
  }
  else{ /* Relative source */
    stored_address=buffer_access(membuf,length,entrypt+1);
    address=entrypt + stored_address;
    op1=buffer_access(membuf,length,address);
  }

  /* Load second operand */
  if( ABSFLG && (opcode & 0x40) ){ /* Absolute source */
    stored_address=buffer_access(membuf,length,entrypt+2);
    address=membuf + stored_address;
    op2=buffer_access(membuf,length,address);
  }
  else{ /* Relative source */
    stored_address=buffer_access(membuf,length,entrypt+2);
    address=entrypt + stored_address;
    op2=buffer_access(membuf,length,address);
  }

  /* Decode operation and execute */
  #if OPCODES
  switch( opcode & 0x03 ){
  case 0x00:
    dest = 0xff-op1;
    break;
  case 0x01:
    dest = op1&op2;
    break;
  case 0x02:
    dest = op1+op2;
    break;
  case 0x03:
    dest = (op1-op2 > 0)? op1-op2: 0;
    break;
  }
  #else
  dest = (op1-op2 > 0)? op1-op2: 0;
  #endif

  /* Deposit destination */
  if( ABSFLG && (opcode & 0x20) ){ /* Absolute destination */
    stored_address=buffer_access(membuf,length,entrypt+3);
    address=membuf + stored_address;
    address=buffer_address(membuf,length,address);
  }
  else{ /* Relative destination */
    stored_address=buffer_access(membuf,length,entrypt+3);
    address=entrypt + stored_address;
    address=buffer_address(membuf,length,address);
  }
  *address=dest;
}

void print_membuf( unsigned char *membuf, unsigned length ){
  unsigned i;

  for( i = 0; i < length; i++ ){
    if( membuf[i] )
      printf( "%02x ", membuf[i] );
    else
      printf( "   " );
    
    if( !((i+1) % 32) ) printf("\n");
  }
  printf("\n");
}

int main( int argc, char *argv[] ){
  unsigned char membuf[MEMSIZ];
  unsigned char *address;
  unsigned i, nnz;

  /* Clear memory */
  for( i = 0; i < MEMSIZ; i ++ )
    membuf[i]= (unsigned char) random();

  while(1) {
    print_membuf(membuf, MEMSIZ);
    for( nnz = 0, i = 0; i < MEMSIZ; i ++ )
      if(membuf[i]) nnz++;
    printf("nnz = %u\n",nnz);
    
    address=membuf+random();
    run_instruction( membuf, MEMSIZ, address );
  }
}
