# Deliverable 3: Experimental Datasets
## EXAMPLE 3: E-Commerce Capability Composition

**Implementation Domain**: Enterprise Microservice & Service Pipeline Architecture  
**Directory**: `EXAMPLE 3 - Ecommerce Capability Composition/`

---

## 1. Dataset Overview

This implementation includes three enterprise-scale benchmark datasets formatted in standardized JSON. Each dataset defines a complete multi-variable business state space, initial checkout configuration, target fulfillment goals, alternative service implementations, and multi-criteria QoS attributes.

| Dataset File | Enterprise Business Domain | State Variables ($d_s$) | Total Capabilities ($|\mathcal{C}|$) | Alternative Implementations | Core Target Goal |
|:---|:---|:---:|:---:|:---:|:---|
| [`ecommerce_purchase_flow.json`](ecommerce_purchase_flow.json) | Purchase Order Fulfillment | 12 | 10 | API vs DB vs GUI | Order Fulfilled & Dispatched |
| [`ecommerce_refund_return_flow.json`](ecommerce_refund_return_flow.json) | Reverse Logistics & RMA Returns | 8 | 7 | Inspection & Banking | Restocking Fee Captured & Refund Reversed |
| [`ecommerce_b2b_wholesale_order.json`](ecommerce_b2b_wholesale_order.json) | B2B Procurement & Freight Logistics | 8 | 8 | Credit Lines & Freight Carrier | Pallets Staged & FTL Freight Dispatched |

---

## 2. Detailed Dataset Specifications

### 2.1 Benchmark 1: `ecommerce_purchase_flow.json`
- **Domain**: Canonical retail purchase checkout, payment processing, inventory allocation, and shipment.
- **State Space Dimensions ($d_s = 12$)**:
  - `user_authenticated`: Customer identity verified.
  - `cart_has_items`: Shopping cart populated with items.
  - `order_created`: Order entity initialized in database.
  - `inventory_validated`: Items reserved in fulfillment warehouse.
  - `payment_method_valid`: Payment token verified with processor.
  - `payment_succeeded`: Funds captured by merchant acquiring bank.
  - `invoice_generated`: Formal PDF tax invoice synthesized.
  - `shipping_label_created`: Carrier tracking number generated.
  - `package_dispatched`: Physical parcel handed to courier.
  - `customer_notified`: Dispatch email and SMS confirmation sent.
  - `order_cancelled`: Negative terminal state.
  - `payment_failed`: Negative terminal state.
- **Initial State $S_I$**: `user_authenticated = true`, `cart_has_items = true`.
- **Goal State $G$**: `order_created = true`, `payment_succeeded = true`, `invoice_generated = true`, `package_dispatched = true`, `customer_notified = true`.
- **Key Capabilities**:
  - `CreateOrder`: Initializes order entity from cart ($C_{\text{time}} = 50\,\text{ms}, C_{\text{money}} = \$0.01$).
  - `ValidateInventory`: Validates stock and decrements warehouse inventory ($C_{\text{time}} = 30\,\text{ms}, C_{\text{money}} = \$0.005$).
  - `ValidatePaymentMethod`: Validates credit card token ($C_{\text{time}} = 40\,\text{ms}, C_{\text{money}} = \$0.005$).
  - `MakePayment_API`: Captures payment via Stripe REST API ($C_{\text{time}} = 150\,\text{ms}, C_{\text{money}} = \$0.02$).
  - `MakePayment_DB`: Competing implementation capturing payment via direct ledger transaction ($C_{\text{time}} = 20\,\text{ms}, C_{\text{money}} = \$0.001$).
  - `GenerateInvoice`: Generates tax invoice PDF ($C_{\text{time}} = 100\,\text{ms}, C_{\text{money}} = \$0.01$).
  - `CreateShippingLabel`: Generates carrier shipping label ($C_{\text{time}} = 60\,\text{ms}, C_{\text{money}} = \$0.005$).
  - `DispatchPackage`: Dispatches package with warehouse courier ($C_{\text{time}} = 120\,\text{ms}, C_{\text{money}} = \$0.05$).
  - `SendNotification`: Dispatches customer confirmation notification ($C_{\text{time}} = 30\,\text{ms}, C_{\text{money}} = \$0.001$).
  - `AuditLog`: Irrelevant distractor capability for experiment validation ($C_{\text{time}} = 5\,\text{ms}, C_{\text{money}} = \$0.0001$).

---

### 2.2 Benchmark 2: `ecommerce_refund_return_flow.json`
- **Domain**: Automated reverse logistics RMA processing and bank fund reversals.
- **State Space Dimensions ($d_s = 8$)**:
  - `return_requested`: Customer submitted RMA request.
  - `package_received_at_hub`: Return parcel scanned at distribution center.
  - `item_inspected_condition`: Optical computer vision condition inspection passed.
  - `restocking_fee_calculated`: Processing fee evaluated.
  - `refund_voucher_issued`: Store credit voucher created.
  - `bank_reversal_completed`: Merchant ACH / card refund captured.
  - `customer_return_closed`: Return process finalized.
  - `item_rejected_damaged`: Item rejected due to customer abuse.
- **Initial State $S_I$**: `return_requested = true`, `package_received_at_hub = true`.
- **Goal State $G$**: `bank_reversal_completed = true`, `customer_return_closed = true`.
- **Capabilities**:
  - `ScanReturnPackage`: High-throughput barcode scanner at intake dock.
  - `InspectItemCondition_Vision`: Automated optical condition assessment.
  - `CalculateRestockingFee`: Dynamic fee deduction engine based on SKU category.
  - `IssueRefundVoucher`: Instant digital credit issuance.
  - `ExecuteBankReversal_ACH`: Overnight low-cost bank clearing.
  - `ExecuteBankReversal_Card`: Instant card refund gateway.
  - `CloseReturnTicket`: Updates ERP ticket status and sends customer email.

---

### 2.3 Benchmark 3: `ecommerce_b2b_wholesale_order.json`
- **Domain**: Enterprise wholesale procurement, credit line underwriting, and pallet freight shipping.
- **State Space Dimensions ($d_s = 8$)**:
  - `b2b_quote_requested`: Buyer submitted RFQ.
  - `corporate_credit_approved`: Dun & Bradstreet risk scoring completed.
  - `tier3_volume_discount_applied`: Bulk tier-3 wholesale discount calculated.
  - `electronic_contract_signed`: DocuSign contract executed.
  - `pallet_inventory_staged`: Full pallet loads retrieved from high-bay racking.
  - `ftl_freight_carrier_assigned`: Full truckload carrier dispatched.
  - `electronic_bol_issued`: Digital Bill of Lading generated.
  - `order_fulfilled`: Procurement cycle complete.
- **Initial State $S_I$**: `b2b_quote_requested = true`.
- **Goal State $G$**: `electronic_bol_issued = true`, `order_fulfilled = true`.
- **Capabilities**:
  - `EvaluateCorporateCredit`: Evaluates buyer trade credit line.
  - `ApplyVolumeDiscount`: Calculates tiered discount structure.
  - `ExecuteDocuSignContract`: Automated legal document workflow.
  - `StagePalletWarehouse`: Automated Guided Vehicles (AGV) stage pallet inventory.
  - `AssignFTLFreight`: Brokerage API assigns logistics carrier.
  - `IssueElectronicBOL`: Generates legal Bill of Lading.
  - `FinalizeB2BOrder`: Closes procurement workflow in SAP ERP.

---

## 3. Dataset JSON Schema

```json
{
  "application": "EcommerceCapabilityComposition",
  "domain": "Microservice Architecture",
  "description": "Workflow description",
  "state_variables": [
    {"name": "var_name", "type": "bool", "description": "Variable description"}
  ],
  "initial_state": {
    "var_name": true
  },
  "goal": {
    "target_var": true
  },
  "capabilities": [
    {
      "id": "CapabilityID",
      "type": "SERVICE",
      "name": "Human Name",
      "preconditions": {"var_name": true},
      "effects": {"var_name": true},
      "inputs": [{"name": "in_data", "type": "UUID"}],
      "outputs": [{"name": "out_data", "type": "UUID"}],
      "qos": {
        "time": 150.0,
        "cost": 0.02,
        "resource": 1.0,
        "risk": 0.001,
        "reliability": 0.999
      },
      "mechanism": "REST POST https://api.service.internal/v1/resource"
    }
  ]
}
```
