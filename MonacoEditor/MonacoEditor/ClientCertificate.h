#pragma once
#include "pch.h"

struct ClientCertificate
{
	ClientCertificate() {};

	ClientCertificate(const ClientCertificate& clientCertificate) {
		Subject = wil::make_cotaskmem_string(clientCertificate.Subject.get());
		DisplayName = wil::make_cotaskmem_string(clientCertificate.DisplayName.get());
		Issuer = wil::make_cotaskmem_string(clientCertificate.Issuer.get());
		ValidFrom = clientCertificate.ValidFrom;
		ValidTo = clientCertificate.ValidTo;
		CertificateKind = clientCertificate.CertificateKind;
	}

	wil::unique_cotaskmem_string Subject;
	wil::unique_cotaskmem_string DisplayName;
	wil::unique_cotaskmem_string Issuer;
	double ValidFrom;
	double ValidTo;
	PCWSTR CertificateKind;
};