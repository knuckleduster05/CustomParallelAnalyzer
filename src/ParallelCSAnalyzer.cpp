#include "ParallelCSAnalyzer.h"
#include "ParallelCSAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

SimpleParallelAnalyzer::SimpleParallelAnalyzer()
:	Analyzer2(),  
	mSettings( new SimpleParallelAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

SimpleParallelAnalyzer::~SimpleParallelAnalyzer()
{
	KillThread();
}

void SimpleParallelAnalyzer::SetupResults()
{
	mResults.reset( new SimpleParallelAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mChipSelectChannel );
}

void SimpleParallelAnalyzer::WorkerThread()
{
	//Store sample rate of analyzer
	mSampleRateHz = GetSampleRate();

	//Set type of arrow to show when putting arrow on clock graphic
	AnalyzerResults::MarkerType clock_arrow;
	if( mSettings->mClockEdge == AnalyzerEnums::NegEdge )
		clock_arrow = AnalyzerResults::DownArrow;
	else
		clock_arrow = AnalyzerResults::UpArrow;

	//Get all data on the clock channel that is available?
	
	mChipSelect = GetAnalyzerChannelData( mSettings->mChipSelectChannel );
	mClock = GetAnalyzerChannelData( mSettings->mClockChannel );
	mData.clear();
	mDataMasks.clear();

	U32 count = mSettings->mDataChannels.size();
	for( U32 i=0; i<count; i++ )
	{
		if( mSettings->mDataChannels[i] != UNDEFINED_CHANNEL )
		{
			mData.push_back( GetAnalyzerChannelData( mSettings->mDataChannels[i] ) );
			mDataMasks.push_back( 1 << i );
			mDataChannels.push_back( mSettings->mDataChannels[i] );
		}
	}


	U32 num_data_lines = mData.size();

	if( mSettings->mChipSelectEdge == AnalyzerEnums::NegEdge )
	{
		if( mChipSelect->GetBitState() == BIT_LOW )
			mChipSelect->AdvanceToNextEdge();
	}else
	{
		if( mChipSelect->GetBitState() == BIT_HIGH )
			mChipSelect->AdvanceToNextEdge();
	}
	


	for( ; ; ) //Wrap the parallel analyzer in a chip select analyzer
	{

	mChipSelect->AdvanceToNextEdge();
	
	mResults->AddMarker( mChipSelect->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mChipSelectChannel );
	
	mClock->AdvanceToAbsPosition(mChipSelect->GetSampleNumber()-1);

	if( mSettings->mClockEdge == AnalyzerEnums::NegEdge )
	{
		if( mClock->GetBitState() == BIT_LOW )
			mClock->AdvanceToNextEdge();
	}else
	{
		if( mClock->GetBitState() == BIT_HIGH )
			mClock->AdvanceToNextEdge();
	}


	mClock->AdvanceToNextEdge();  //this is the data-valid edge

	Frame last_frame;
	bool added_last_frame = false;
	for( ; ; )
	{
		if(mSettings->mChipSelectEdge == AnalyzerEnums::NegEdge ) {
			if( mChipSelect->GetBitState() == BIT_HIGH)
				mChipSelect->AdvanceToNextEdge(); //Skip all between non cs
				break;
		} else {
			if( mChipSelect->GetBitState() == BIT_LOW)
				mChipSelect->AdvanceToNextEdge(); //Skip all between non cs
				break;
		}
		
		//here we found a rising edge at the mark. Add images to mResults
		U64 sample = mClock->GetSampleNumber();
		mResults->AddMarker( sample, clock_arrow, mSettings->mClockChannel );

		U16 result = 0;

		for( U32 i=0; i<num_data_lines; i++ )
		{
			mData[i]->AdvanceToAbsPosition( sample );
			if( mData[i]->GetBitState() == BIT_HIGH )
			{
				result |= mDataMasks[i];
			}
			mResults->AddMarker( sample, AnalyzerResults::Dot, mDataChannels[i] );
		}	

		Frame frame;
		frame.mData1 = result;
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = sample;

		if( added_last_frame || mClock->DoMoreTransitionsExistInCurrentData() ) //if this thread ever gets before the data worker thread then this condition may hit before the end of the dataset which will only cause a problem if the clock changed between packets. The last frame will use the old clock rate for ending sample
		{
			//wait until there is another rising edge so we know the ending sample
			mClock->AdvanceToNextEdge();

			if( mClock->DoMoreTransitionsExistInCurrentData() )
			{
				mClock->AdvanceToNextEdge();  //this is the data-valid edge
				added_last_frame = false;
			}
			else
				added_last_frame = true;

			frame.mEndingSampleInclusive = mClock->GetSampleNumber() - 1;
		}
		else
		{
			frame.mEndingSampleInclusive = frame.mStartingSampleInclusive + ( last_frame.mEndingSampleInclusive - last_frame.mStartingSampleInclusive);
			added_last_frame = true;
		}

		last_frame = frame;
		//finally, add the frame
		mResults->AddFrame( frame );
		mResults->CommitResults();
		ReportProgress( frame.mEndingSampleInclusive );
	}
	}
}

bool SimpleParallelAnalyzer::NeedsRerun()
{
	return false;
}

U32 SimpleParallelAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 SimpleParallelAnalyzer::GetMinimumSampleRateHz()
{
	return 1000000;
}

const char* SimpleParallelAnalyzer::GetAnalyzerName() const
{
	return "ParallelCS";
}

const char* GetAnalyzerName()
{
	return "Parallel w/ Chip Select";
}

Analyzer* CreateAnalyzer()
{
	return new SimpleParallelAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
