#include "ItemMessageWidget.h"

void UItemMessageWidget::SetMessage(FString Message)
{
    if (MessageText)
    {
        MessageText->SetText(FText::FromString(Message));
    }
}