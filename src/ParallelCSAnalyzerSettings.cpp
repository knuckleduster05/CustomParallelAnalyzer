#include "ParallelCSAnalyzerSettings.h"
#include <AnalyzerHelpers.h>
#include <stdio.h>


#pragma warning( disable : 4996 ) //warning C4996: 'sprintf': This function or variable may be unsafe

SimpleParallelAnalyzerSettings::SimpleParallelAnalyzerSettings()
:	
	mChipSelectChannel( UNDEFINED_CHANNEL ),
	mChipSelectEdge( AnalyzerEnums::PosEdge ),
	mClockChannel( UNDEFINED_CHANNEL ),
	mClockEdge( AnalyzerEnums::PosEdge )
{
	U32 count = 15;
	for( U32 i=0; i<count; i++ )
	{
		mDataChannels.push_back( UNDEFINED_CHANNEL );
		AnalyzerSettingInterfaceChannel* data_channel_interface = new AnalyzerSettingInterfaceChannel();
		
		char text[64];
		sprintf( text, "D%d", i );

		data_channel_interface->SetTitleAndTooltip( text, text );
		data_channel_interface->SetChannel( mDataChannels[i] );
		data_channel_interface->SetSelectionOfNoneIsAllowed( true );

		mDataChannelsInterface.push_back( data_channel_interface );
	}

	mChipSelectChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mChipSelectChannelInterface->SetTitleAndTooltip( "CS", "Chip Select" );
	mChipSelectChannelInterface->SetChannel( mChipSelectChannel );

	mChipSelectEdgeInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mChipSelectEdgeInterface->SetTitleAndTooltip( "Chip Select Edge", "Select if CS is active after a rising or falling edge" );
	mChipSelectEdgeInterface->AddNumber( AnalyzerEnums::PosEdge, "Rising edge", "" );
	mChipSelectEdgeInterface->AddNumber( AnalyzerEnums::NegEdge, "Falling edge", "" );
	mChipSelectEdgeInterface->SetNumber( mChipSelectEdge );

	mClockChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mClockChannelInterface->SetTitleAndTooltip( "Clock", "Clock" );
	mClockChannelInterface->SetChannel( mClockChannel );

	mClockEdgeInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mClockEdgeInterface->SetTitleAndTooltip( "Clock State", "Define whether the data is valid on Clock rising or falling edge" );
	mClockEdgeInterface->AddNumber( AnalyzerEnums::PosEdge, "Rising edge", "" );
	mClockEdgeInterface->AddNumber( AnalyzerEnums::NegEdge, "Falling edge", "" );
	mClockEdgeInterface->SetNumber( mClockEdge );



	for( U32 i=0; i<count; i++ )
	{
		AddInterface( mDataChannelsInterface[i] );
	}

	AddInterface( mChipSelectChannelInterface.get() );
	AddInterface( mChipSelectEdgeInterface.get() );
	AddInterface( mClockChannelInterface.get() );
	AddInterface( mClockEdgeInterface.get() );

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	for( U32 i=0; i<count; i++ )
	{
		char text[64];
		sprintf( text, "D%d", i );
		AddChannel( mDataChannels[i], text, false );
	}

	AddChannel( mChipSelectChannel, "CS", false);
	AddChannel( mClockChannel, "Clock", false );
}

SimpleParallelAnalyzerSettings::~SimpleParallelAnalyzerSettings()
{
	U32 count = mDataChannelsInterface.size();
	for( U32 i=0; i<count; i++ )
		delete mDataChannelsInterface[i];
}

bool SimpleParallelAnalyzerSettings::SetSettingsFromInterfaces()
{
	U32 count = mDataChannels.size();
	U32 num_used_channels = 0;
	for( U32 i=0; i<count; i++ )
	{
		if( mDataChannelsInterface[i]->GetChannel() != UNDEFINED_CHANNEL )
			num_used_channels++;
	}

	if( num_used_channels == 0 )
	{
		SetErrorText( "Please select at least one channel to use in the parallel bus" );
		return false;
	}

	for( U32 i=0; i<count; i++ )
	{
		mDataChannels[i] = mDataChannelsInterface[i]->GetChannel();
	}

	mChipSelectChannel = mChipSelectChannelInterface->GetChannel();
	mChipSelectEdge = AnalyzerEnums::EdgeDirection( U32( mChipSelectEdgeInterface->GetNumber() ) );
	mClockChannel = mClockChannelInterface->GetChannel();
	mClockEdge = AnalyzerEnums::EdgeDirection( U32( mClockEdgeInterface->GetNumber() ) );

	ClearChannels();
	for( U32 i=0; i<count; i++ )
	{
		char text[64];
		sprintf( text, "D%d", i );
		AddChannel( mDataChannels[i], text, mDataChannels[i] != UNDEFINED_CHANNEL );
	}

	AddChannel( mChipSelectChannel, "CS", true );
	AddChannel( mClockChannel, "Clock", true );

	return true;
}

void SimpleParallelAnalyzerSettings::UpdateInterfacesFromSettings()
{
	U32 count = mDataChannels.size();
	for( U32 i=0; i<count; i++ )
	{
		mDataChannelsInterface[i]->SetChannel( mDataChannels[i] );
	}

	mChipSelectChannelInterface->SetChannel( mChipSelectChannel );
	mChipSelectEdgeInterface->SetNumber( mChipSelectEdge );
	mClockChannelInterface->SetChannel( mClockChannel );
	mClockEdgeInterface->SetNumber( mClockEdge );
}

void SimpleParallelAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	U32 count = mDataChannels.size();

	for( U32 i=0; i<count; i++ )
	{
		text_archive >> mDataChannels[i];
	}

	text_archive >> mChipSelectChannel;
	text_archive >> *(U32*) &mChipSelectEdge;
	text_archive >> mClockChannel;
	text_archive >> *(U32*)&mClockEdge;

	ClearChannels();
	for( U32 i=0; i<count; i++ )
	{
		char text[64];
		sprintf( text, "D%d", i );
		AddChannel( mDataChannels[i], text, mDataChannels[i] != UNDEFINED_CHANNEL );
	}

	AddChannel( mChipSelectChannel, "CS", true );
	AddChannel( mClockChannel, "Clock", true );

	UpdateInterfacesFromSettings();
}

const char* SimpleParallelAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

		U32 count = mDataChannels.size();

	for( U32 i=0; i<count; i++ )
	{
		text_archive << mDataChannels[i];
	}

	text_archive << mChipSelectChannel;
	text_archive << mChipSelectEdge;
	text_archive << mClockChannel;
	text_archive << mClockEdge;

	return SetReturnString( text_archive.GetString() );
}
