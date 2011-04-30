/*
** Copyright (C) 2009 Tadej Borovšak <tadeboro@gmail.com>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "img_sox.h"

/* Global data - libsox has no means to pass data
 * to effect handlers */
static ImgThreadData *global;

static int
output_flow( sox_effect_t       *effp,
			 sox_sample_t const *ibuf,
			 sox_sample_t       *obuf,
			 size_t             *isamp,
			 size_t             *osamp )
{
	if( ! g_atomic_int_get( global->sox_flags ) )
	{
		sox_write( global->output, ibuf, *isamp );
		*osamp = 0;

		return( SOX_SUCCESS );
	}

	return( SOX_EOF );
}

static sox_effect_handler_t const *
custom_output( void )
{
	static sox_effect_handler_t h = { "custom_out",  /* Name */
									  NULL,          /* Usage */
									  SOX_EFF_MCHAN, /* Flags */
									  NULL,          /* Getopts func */
									  NULL,          /* Start func */
									  output_flow,   /* Flow func */
									  NULL,          /* Drain func */
									  NULL,          /* Stop func */
									  NULL,          /* Kill func */
									  0 };           /* Private data size */

	return( &h );
}

static int
input_drain( sox_effect_t *effp,
			 sox_sample_t *obuf,
			 size_t       *osamp )
{
	size_t read;

	*osamp -= *osamp % effp->out_signal.channels;
	read = sox_read( global->input, obuf, *osamp );

	if( ! read )
	{
		sox_close( global->input );
		global->input = NULL;

		if( global->current_input < global->no_files )
		{
			global->input = sox_open_read( global->files[global->current_input],
										   NULL, NULL, NULL );
			global->current_input++;
			read = sox_read( global->input, obuf, *osamp );
		}
		else
		{
			*osamp = 0;
			return( SOX_EOF );
		}
	}

	*osamp = read;
	return( SOX_SUCCESS );
}

static sox_effect_handler_t const *
custom_input( void )
{
	static sox_effect_handler_t h = { "custom_in",   /* Name */
									  NULL,          /* Usage */
									  SOX_EFF_MCHAN, /* Flags */
									  NULL,          /* Getopts func */
									  NULL,          /* Start func */
									  NULL,          /* Flow func */
									  input_drain,   /* Drain func */
									  NULL,          /* Stop func */
									  NULL,          /* Kill func */
									  0 };           /* Private data size */

	return( &h );
}

gpointer
img_produce_audio_data( ImgThreadData *data )
{
	sox_effects_chain_t *chain;
	sox_effect_t        *effect;
	sox_signalinfo_t     signal;
	gchar               *fargs[] = { "t",   /* Logarithmic fade */
									 "0",   /* 5 s fade-in */
									 "",    /* Placeholder for total lenght */
									 "5" }; /* 5 s fade-out */
	fargs[2] = g_strdup_printf( "%d", data->length );

	/* Initialize thread private part of the structure */
	data->current_input = 1;
	data->input = sox_open_read( data->files[0], NULL, NULL, NULL );
	signal = data->input->signal;

	global = data;

	/* Output handler */
	data->output = sox_open_write( data->fifo, &data->input->signal,
								   NULL, "flac", NULL, NULL );

	/* Effect chain */
	chain = sox_create_effects_chain( &data->input->encoding,
									  &data->output->encoding );

	effect = sox_create_effect( custom_input() );
	sox_add_effect( chain, effect, &signal, &signal );

	effect = sox_create_effect( sox_find_effect( "fade" ) );
	sox_effect_options( effect, 4, fargs );
	sox_add_effect( chain, effect, &signal, &signal );

	effect = sox_create_effect( custom_output() );
	sox_add_effect( chain, effect, &signal, &signal );

	/* Run chain */
#if SOX_LIB_VERSION_CODE == SOX_LIB_VERSION( 14, 2, 0 )
	sox_flow_effects( chain, NULL );
#else
	sox_flow_effects( chain, NULL, NULL );
#endif

	/* Inform parent that we're finished */
	g_atomic_int_set( data->sox_flags, 2 );

	/* Cleanup */
	sox_delete_effects_chain( chain );
	if( data->input )
		sox_close( data->input );
	sox_close( data->output );
	g_free( fargs[2] );
	g_slice_free( ImgThreadData, data );
	global = NULL;

	return( NULL );
}
