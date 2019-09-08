/*
 *  The Games: Winter/Summer Challenge - (1991-'92) series by Accolade,
 *  game data extracting tool
 *
 *  Copyright (c) 2001-2019 Gerard P.Swiderski
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#define MB_SIGNATURE        0x424d
#define EXE_SIGNATURE       0x5a4d

#define W_ALIGN( _ofs )     ( ( _ofs ) + ( ( _ofs ) % 2 ) )
#define MAX( _a, _b )       ( ( _a ) > ( _b ) ? ( _a ) : ( _b ) )

struct exe_mini_head {
    unsigned short signature;   /* MZ */
    unsigned short bytes_in_last_block;
    unsigned short blocks_in_file;
};

struct mb_flt_entry {
    size_t len;
    size_t ofs;                 /* word aligned */
    char name[ 14 ];
};

struct mb_head {
    unsigned short signature;   /* MB */
    unsigned short nentries;
    struct mb_flt_entry *flt;
};

int main( int argc, const char *argv[] )
{
    FILE *fin, *fout;
    int i, j;
    ssize_t flen;
    struct exe_mini_head mz;
    struct mb_head mb = { .flt = NULL };
    size_t extra_data_start = 0, maxlen = 0;
    char str[ 128 ] = { 0 };
    void *buf = NULL;

    if ( argc < 2 ) {
        printf( "Extract game data into directory named 'out/'\n" );
        printf( "usage: %s <winter.exe|summer.exe>\n", argv[ 0 ] );
        return 0;
    }

    fin = fopen( argv[ 1 ], "rb" );

    if ( !fin )
        goto error;

    if ( fread( &mz.signature, 1, 2, fin ) != 2 )
        goto error;

    if ( mz.signature != EXE_SIGNATURE ) {
        printf( "not exe file\n" );
        goto error;
    }
    if ( fread( &mz.bytes_in_last_block, 1, 2, fin ) != 2 )
        goto error;
    if ( fread( &mz.blocks_in_file, 1, 2, fin ) != 2 )
        goto error;

    extra_data_start = mz.blocks_in_file * 512L;
    if ( mz.bytes_in_last_block != 0 )
        extra_data_start -= 512L - mz.bytes_in_last_block;

    if ( fseek( fin, 0L, SEEK_END ) < 0 )
        goto error;

    if ( ( flen = ftell( fin ) ) < 0 )
        goto error;

    if ( extra_data_start >= flen ) {
        printf( "no game data in exe file\n" );
        goto error;
    }

    if ( fseek( fin, extra_data_start, SEEK_SET ) < 0 )
        goto error;

    if ( fread( &mb.signature, 1, 2, fin ) != 2 )
        goto error;

    if ( mb.signature != MB_SIGNATURE ) {
        printf( "not game file\n" );
        goto error;
    }

    if ( fread( &mb.nentries, 1, 2, fin ) != 2 )
        goto error;

    if ( !( mb.flt = malloc( mb.nentries * sizeof( *mb.flt ) ) ) )
        goto failed;

    for ( i = 0; i < mb.nentries; i++ ) {
        struct mb_flt_entry *p = &mb.flt[ i ];

        if ( fread( &p->len, 1, 4, fin ) != 4 )
            goto failed;
        if ( fread( &p->ofs, 1, 4, fin ) != 4 )
            goto failed;
        if ( p->ofs != W_ALIGN( p->ofs ) )
            goto failed;
        if ( fread( &p->name, 1, 14, fin ) != 14 )
            goto failed;

        for ( j = 0; j < 14 && p->name[ j ]; j++ )
            p->name[ j ] -= 0x60;

        if ( j > 12 )
            goto failed;

        maxlen = MAX( maxlen, p->len );
    }

    if ( !( buf = malloc( maxlen ) ) )
        goto failed;

    for ( i = 0; i < mb.nentries; i++ ) {
        fseek( fin, mb.flt[ i ].ofs, SEEK_SET );

        if ( fread( buf, 1, mb.flt[ i ].len, fin ) != mb.flt[ i ].len )
            goto failed;

        printf( "name='%s',offset=%d,len=%d\n", mb.flt[ i ].name,
                mb.flt[ i ].ofs,
                mb.flt[ i ].len
                );

        snprintf( str, 127, "out/%s", mb.flt[ i ].name );

        if ( !( fout = fopen( str, "w" ) ) )
            goto failed;

        if ( fwrite( buf, 1, mb.flt[ i ].len, fout ) != mb.flt[ i ].len )
            goto failed;

        fclose( fout );
    }

    printf( "mb_offset=%d,nelem=%d,maxlen=%d\n",
            extra_data_start, mb.nentries, maxlen );

    free( buf );
    free( mb.flt );
    fclose( fin );

    return 0;

failed:
    free( buf );
    free( mb.flt );

error:
    fclose( fin );
    printf( "error occured\n" );

    return -1;
}
